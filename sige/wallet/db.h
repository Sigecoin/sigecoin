// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __wallet_db_h__
#define __wallet_db_h__

#include "clientversion.h"
#include "serialize.h"
#include "streams.h"
#include "sync.h"
#include "version.h"

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

#include <boost/filesystem/path.hpp>
#include <liblmdb/lmdb.h>

static const int ChecksumOfDBPtr = 12345;
static const unsigned int DEFAULT_WALLET_DBLOGSIZE = 100;
static const bool DEFAULT_WALLET_PRIVDB = true;

class CDB;
class CDBTransact;
/////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBEnv
{
    friend class CDBTransact;
private:

    int  checksum_;
    bool fDbEnvInit;
    bool fMockDb;
    // Don't change into boost::filesystem::path, as that can result in
    // shutdown problems/crashes caused by a static initialized internal pointer.
    std::string strPath;
    std::mutex lock;
    std::condition_variable open_notify;
    unsigned open_transactions;
    std::atomic_uint transaction_iteration;
    std::condition_variable resize_notify;
    std::atomic_bool resizing;
    std::function <void()> sizing_action;

    void EnvShutdown();
    void handle_environment_sizing();
    void add_transaction();
    void remove_transaction();
    bool isRefValid() const { return 0 == memcmp(&checksum_,&ChecksumOfDBPtr,4); }

public:
    mutable CCriticalSection cs_db;
    MDB_env* environment;
    std::map<std::string, int> mapFileUseCount;
    std::multimap<std::string, CDB*> mapDb;

    CDBEnv();
    ~CDBEnv();
    void Reset();

    void MakeMock();
    bool IsMock() { return fMockDb; }

    /**
     * Verify that database file strFile is OK. If it is not,
     * call the callback to try to recover.
     * This must be called BEFORE strFile is opened.
     * Returns true if strFile is OK.
     */
    enum VerifyResult { VERIFY_OK,
                        RECOVER_OK,
                        RECOVER_FAIL };
    /**
     * Salvage data from a file that Verify says is bad.
     * fAggressive sets the DB_AGGRESSIVE flag (see berkeley DB->verify() method documentation).
     * Appends binary key/value pairs to vResult, returns true if successful.
     * NOTE: reads the entire database into memory, so cannot be used
     * for huge databases.
     */
    typedef std::pair<std::vector<unsigned char>, std::vector<unsigned char> > KeyValPair;

    bool Open(const boost::filesystem::path& path);
    void Close();
    void Flush(bool fShutdown);
    void CheckpointLSN(const std::string& name);
    void CloseDb(const std::string& name);

    /**
    * Returns true if opened database removed with success 
    * Otherwise database may be shared with another thread (use CloseDb before to obtain the reference counter)
    */
    bool RemoveDb(const std::string& name);

    CDBTransact* TxnBegin(bool readonly);
};

extern CDBEnv bitdb;

/////////////////////////////////////////////////////////////////////////////////////////////////////
class CDBTransact
{
    friend class CDBEnv;
private:
    CDBTransact(CDBEnv& parent, MDB_txn* transaction) : parent_(parent), transaction_(transaction) {}
    ~CDBTransact() {}
    MDB_txn* transaction_;
    CDBEnv& parent_;
public:
    MDB_txn* get() const { return transaction_; };
    void release() {
        if (parent_.isRefValid())
            parent_.remove_transaction();
        delete this;
    }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// RAII class that provides access to a Berkeley database
class CDB
{
    friend class CDBEnv;

protected:
    MDB_dbi db_;
    std::string strName;
    CDBTransact* activeTxn;
    bool fReadOnly;
    bool fFlushOnClose;

    explicit CDB(const std::string& name, const char* pszMode = "r+", bool fFlushOnCloseIn=true);
    ~CDB();

public:
    void Flush();
    /**
    * Close returns ref counter opened database in another threads
    */
    int  Close();

private:
    CDB(const CDB&);

protected:

    MDB_txn* getTx()
    {
        if (activeTxn == NULL) TxnBegin();
        return activeTxn ? activeTxn->get() : NULL;
    }

    template <typename K, typename T>
    bool Read(const K& key, T& value)
    {
        if (db_ == 0)
            return false;

        // Key
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(1000);
        ssKey << key;
        MDB_val datKey = { ssKey.size(), ssKey.data() };
        MDB_val constData = {0};
        auto status = mdb_get(getTx(), db_, &datKey, &constData);
        memset(datKey.mv_data, 0, datKey.mv_size);
        if (constData.mv_size > 0 && constData.mv_data != NULL)
        {
            try {
                CDataStream ssValue((char*)constData.mv_data, (char*)constData.mv_data + constData.mv_size, SER_DISK, CLIENT_VERSION);
                ssValue >> value;
            }
            catch (const std::exception&) { return false; }
        }
        return (status == 0);
    }

    template <typename K, typename T> bool Write(const K& key, const T& value, bool fOverwrite = true)
    {
        if (db_ == 0)
            return false;
        if (fReadOnly == true)
            assert(!"Write called on database in read-only mode");

        // Key
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(1000);
        ssKey << key;
        MDB_val datKey = { ssKey.size(), ssKey.data() };

        // Value
        CDataStream ssValue(SER_DISK, CLIENT_VERSION);
        ssValue.reserve(10000);
        ssValue << value;
        MDB_val datValue = { ssValue.size(), ssValue.data() };

        // Write
        auto status = mdb_put(getTx(), db_, &datKey, &datValue, fOverwrite ? 0 : MDB_NOOVERWRITE);

        // Clear memory in case it was a private key
        memset(datKey.mv_data, 0, datKey.mv_size);
        memset(datValue.mv_data, 0, datValue.mv_size);
        return (status == 0);
    }

    template <typename K>
    bool Erase(const K& key)
    {
        if (db_ == 0)
            return false;
        if (fReadOnly)
            assert(!"Erase called on database in read-only mode");

        // Key
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(1000);
        ssKey << key;
        MDB_val datKey = { ssKey.size(), ssKey.data() };

        // Erase
        auto status = mdb_del(getTx(), db_, &datKey, NULL);

        // Clear memory
        memset(datKey.mv_data, 0, datKey.mv_size);
        return (status == 0 || status == MDB_NOTFOUND);
    }

    template <typename K>
    bool Exists(const K& key)
    {
        if (db_ == 0)
            return false;

        // Key
        CDataStream ssKey(SER_DISK, CLIENT_VERSION);
        ssKey.reserve(1000);
        ssKey << key;
        MDB_val datKey = { ssKey.size(), ssKey.data() };

        // Exists
        MDB_val constData = { 0 };
        auto status = mdb_get(getTx(), db_, &datKey, &constData);

        // Clear memory
        memset(datKey.mv_data, 0, datKey.mv_size);
        return (status == 0);
    }

    MDB_cursor* GetCursor()
    {
        if (0 == db_)
            return NULL;
        MDB_cursor* pcursor = NULL;
        auto status = mdb_cursor_open(getTx(), db_, &pcursor);
        return (status == 0 ? pcursor : NULL);
    }

    int ReadAtCursor(MDB_cursor* pcursor, CDataStream& ssKey, CDataStream& ssValue, bool setRange = false)
    {
        // Read at cursor
        MDB_cursor_op op = MDB_NEXT;
        MDB_val datKey = {0};
        if (setRange) {
            datKey = { ssKey.size(), ssKey.data() };
            op = MDB_SET_RANGE;
        }

        MDB_val datValue = {0};
        auto status = mdb_cursor_get(pcursor, &datKey, &datValue, op);
        if (status != 0)
            return status;
        else if (datKey.mv_size == 0 || datValue.mv_size == 0 || datKey.mv_data == NULL  || datValue.mv_data == NULL)
            return 99999;

        // Convert to streams
        ssKey.SetType(SER_DISK);
        ssKey.clear();
        ssKey.write((char*)datKey.mv_data, datKey.mv_size);
        ssValue.SetType(SER_DISK);
        ssValue.clear();
        ssValue.write((char*)datValue.mv_data, datValue.mv_size);
        return 0;
    }

public:
    bool TxnBegin()
    {
        if (bitdb.environment == NULL || activeTxn)
            return false;
        CDBTransact* ptxn = bitdb.TxnBegin(fReadOnly);
        if (!ptxn)
            return false;
        activeTxn = ptxn;
        return true;
    }

    bool TxnCommit()
    {
        if (db_ == 0 || !activeTxn)
            return false;
        auto status = mdb_txn_commit(activeTxn->get());
        activeTxn->release();
        activeTxn = NULL;
        return (status == 0);
    }

    bool TxnAbort()
    {
        if (db_ == 0 || !activeTxn)
            return false;
        mdb_txn_abort(activeTxn->get());
        activeTxn->release();
        activeTxn = NULL;
        return true;
    }

    bool ReadVersion(int& nVersion)
    {
        nVersion = 0;
        return Read(std::string("version"), nVersion);
    }

    bool WriteVersion(int nVersion)
    {
        return Write(std::string("version"), nVersion);
    }
};

#endif  /* __wallet_db_h__ */
