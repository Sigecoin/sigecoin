// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet/db.h"

#include "addrman.h"
#include "hash.h"
#include "protocol.h"
#include "util.h"
#include "utilstrencodings.h"

#include <stdint.h>

#ifndef WIN32
#include <sys/stat.h>
#endif

#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/version.hpp>

using namespace std;


//
// CDB
//

CDBEnv bitdb;

void CDBEnv::EnvShutdown()
{
    if (!fDbEnvInit)
        return;

    mdb_env_close(environment);
    fDbEnvInit = false;
    environment = NULL;

/*
!!! possible will use FS remove instead DbEnv::remove
    if (!fMockDb)
        DbEnv((u_int32_t)0).remove(strPath.c_str(), 0);
*/
}

void CDBEnv::Reset()
{
    if (fDbEnvInit || environment != NULL) mdb_env_close(environment);

    environment = NULL;
    auto status = mdb_env_create(&environment);
    if (status == 0)
    {
        status = mdb_env_set_maxdbs(environment, 128);
        if (status == 0)
        {
            status = mdb_env_set_mapsize(environment, 0);
            if (status == 0)
            {
                fDbEnvInit = false;
                fMockDb = false;
            }
        }
    }
    
    if (status != 0)
    {
        if (environment != NULL) mdb_env_close(environment);
        environment = NULL;
    }
}

CDBEnv::CDBEnv() : 
    environment(NULL),
    resizing(false),
    sizing_action([this]() { handle_environment_sizing(); })
{
    Reset();
    checksum_ = ChecksumOfDBPtr;
}

CDBEnv::~CDBEnv()
{
    EnvShutdown();
}

void CDBEnv::Close()
{
    EnvShutdown();
}

bool CDBEnv::Open(const boost::filesystem::path& pathIn)
{
    if (fDbEnvInit)
        return true;

    boost::this_thread::interruption_point();

    strPath = pathIn.string();
    boost::filesystem::path pathLogDir = pathIn / "database";
    TryCreateDirectory(pathLogDir);
    boost::filesystem::path pathErrorFile = pathLogDir / "db.log";
    LogPrintf("CDBEnv::Open: LogDir=%s ErrorFile=%s\n", pathLogDir.string(), pathErrorFile.string());
    boost::filesystem::path pathDbFile = pathLogDir / "wallet.mdb";

/*    
!!! not used
    bool privat = false;
    if (GetBoolArg("-privdb", DEFAULT_WALLET_PRIVDB))
        privat = true;
*/
    auto status = mdb_env_open(environment, pathDbFile.string().c_str(), MDB_NOSUBDIR, 0600);
    if (status != 0)
        return error("CDBEnv::Open: Error %d opening database environment\n", status);

    fDbEnvInit = true;
    fMockDb = false;
    return true;
}

void CDBEnv::handle_environment_sizing()
{
    MDB_stat stats;
    mdb_env_stat(environment, &stats);
    MDB_envinfo info;
    mdb_env_info(environment, &info);
    double used_space(info.me_last_pgno * stats.ms_psize);
    double needed_space(used_space * 1.25);
    size_t increments_needed((needed_space / /*database_size_increment*/256 * 1024 * 1024) + 1);
    size_t environment_size(increments_needed * /*database_size_increment*/256 * 1024 * 1024);
    if (info.me_mapsize < environment_size)
    {
        if (!resizing.exchange(true))
        {
            std::unique_lock <std::mutex> lock_l(lock);
            while (open_transactions > 0)
            {
                open_notify.wait(lock_l);
            }
            mdb_env_set_mapsize(environment, environment_size);
            resizing = false;
            resize_notify.notify_all();
        }
    }
}

void CDBEnv::add_transaction()
{
    // Minimize I/O from checking if we need to resive
    // Too high and the enviroment might overflow
    // Too low and we're making lots of trips to the disk
    if ((transaction_iteration++ % /*database_check_interval*/256) == 0)
    {
        sizing_action();
    }
    std::unique_lock <std::mutex> lock_l(lock);
    // Wait for any resizing operations to complete
    while (resizing)
    {
        resize_notify.wait(lock_l);
    }
    ++open_transactions;
}

void CDBEnv::remove_transaction()
{
    std::lock_guard <std::mutex> lock_l(lock);
    --open_transactions;
    open_notify.notify_all();
}

CDBTransact* CDBEnv::TxnBegin(bool readonly)
{
    CDBTransact* transact = NULL;
    MDB_txn* ptxn = NULL;
    auto status = mdb_txn_begin(environment, NULL/*parent_a*/, readonly ? MDB_RDONLY : 0, &ptxn);
    if (status == 0 && ptxn != NULL) 
    {
        add_transaction();
        transact = new CDBTransact(*this, ptxn);
    }
    return transact;
}

void CDBEnv::MakeMock()
{
    if (fDbEnvInit)
        throw runtime_error("CDBEnv::MakeMock: Already initialized");

    const boost::filesystem::path& pathIn = ".";
    strPath = pathIn.string();
    boost::filesystem::path pathLogDir = pathIn / "mock";
    TryCreateDirectory(pathLogDir);
    boost::filesystem::path pathDbFile = pathLogDir / "mock.mdb";
    boost::filesystem::path pathErrorFile = pathLogDir / "db_mock.log";
    LogPrintf("CDBEnv::Open: LogDir=%s ErrorFile=%s\n", pathLogDir.string(), pathErrorFile.string());

    auto status = mdb_env_open(environment, pathDbFile.string().c_str(), MDB_NOSUBDIR, 0600);
    if (status != 0)
        throw runtime_error(strprintf("CDBEnv::MakeMock: Error %d opening database environment.", status));

    fDbEnvInit = true;
    fMockDb = true;
}

void CDBEnv::CheckpointLSN(const std::string& name)
{
    auto dbIt = mapDb.find(name);
    if (dbIt != mapDb.end())
    {
        CDB* ptr = dbIt->second;
        int refc = ptr->Close();
        if (!ptr->fFlushOnClose)
            ptr->Flush();

        if (refc == 0) {
            delete ptr;
            mapDb.erase(dbIt);
        }
    }
}

CDB::CDB(const std::string& name, const char* pszMode, bool fFlushOnCloseIn) : strName(name), db_(0), activeTxn(NULL), fReadOnly(false)
{
    const char* ptr = strchr(pszMode, '+');
    if (ptr == NULL) {
        ptr = strchr(pszMode, 'w');
        fReadOnly = ptr == NULL;
    }

    int ret;
    fFlushOnClose = fFlushOnCloseIn;
    if (strName.empty())
        return;

    bool fCreate = strchr(pszMode, 'c') != NULL;
    unsigned int nFlags = MDB_DUPSORT;
    if (fCreate)
        nFlags |= MDB_CREATE;

    {
        LOCK(bitdb.cs_db);
        if (!bitdb.Open(GetDataDir()))
            throw runtime_error("CDB: Failed to open database environment.");

        strName = name;
        auto it = bitdb.mapDb.find(strName);
        if (it != bitdb.mapDb.cend() && bitdb.mapFileUseCount.find(strName) != bitdb.mapFileUseCount.end())
        {
            ++bitdb.mapFileUseCount[strName];
            db_ = it->second->db_;
            activeTxn = it->second->activeTxn;
        }
        else
        {
            auto status = mdb_dbi_open(getTx(), strName.c_str(), nFlags, &db_);
            
            if (status != 0)
            {
                db_ = 0;
                strName = "";
                throw runtime_error(strprintf("CDB: Error %d, can't open database %s", ret, strName));
            }

            if (fCreate && !Exists(string("version"))) 
            {
                bool fTmp = fReadOnly;
                fReadOnly = false;
                WriteVersion(CLIENT_VERSION);
                fReadOnly = fTmp;
            }
            ++bitdb.mapFileUseCount[strName];
        }
        bitdb.mapDb.insert(std::multimap<std::string, CDB*>::value_type(strName, this));
    }
}

CDB::~CDB()
{
    Close();
    // remove DB object from global map
    auto It = bitdb.mapDb.lower_bound(strName);
    for(; It != bitdb.mapDb.end() && It->first == strName; ++It)
        if (It->second == this)
        {
            bitdb.mapDb.erase(It);
            break;
        }
    
}

void CDB::Flush()
{
    if (activeTxn == NULL) return;

    // Flush database activity from memory pool to disk log
    auto status = mdb_env_sync(bitdb.environment, 1);
    if (status != 0) LogPrintf("CDBEnv::Flush: cannot flush data for database %s, error =%d\n", strName, status);
}

int CDB::Close()
{
    if (db_ == 0) return 0;

    if (fFlushOnClose)
        Flush();

    LOCK(bitdb.cs_db);
    int refc = bitdb.mapFileUseCount[strName];
    if (refc < 2)
    {
        if (activeTxn != NULL && fFlushOnClose)
            TxnCommit();
        else if (activeTxn != NULL)
            TxnAbort();
        mdb_dbi_close(bitdb.environment, db_);
        bitdb.mapFileUseCount[strName] = 0;
    }
    else
    {
        bitdb.mapFileUseCount[strName] = --refc;
    }

    db_ = 0;
    return refc;
}

void CDBEnv::CloseDb(const string& name)
{
    int refc = 0;
    {
        LOCK(cs_db);
        auto dbIt = mapDb.find(name);
        if (dbIt != mapDb.cend())
        {
            refc = mapFileUseCount[name];
            if (refc > 0) mapFileUseCount[name] = --refc;

            // Close the database handle
            refc = dbIt->second->Close();
            if (refc == 0) {
                delete dbIt->second;
                bitdb.mapDb.erase(name);
            }
        }
    }
}

bool CDBEnv::RemoveDb(const string& name)
{
    CDB* db = NULL;
    bool notfound = false;
    {
        LOCK(cs_db);
        auto dbIt = mapDb.find(name);
        notfound = (dbIt == mapDb.cend());
        if (!notfound)
        {
            db = dbIt->second;
        }
    }

    if (db == NULL)
    {
        try {
            db = new CDB(name);
            auto dbIt = mapDb.find(name);
            if (dbIt == mapDb.cend()) {
                throw runtime_error(strprintf("CDBEnv::RemoveDb: cannot remove database %s, internal error", name));
            }
        }
        catch (const runtime_error& ex)  
        {
            LogPrintf("%s\n", ex.what());
            if (db && 0 == db->Close()) {
                delete db;
                mapDb.erase(name);
            }
            return false; 
        }
        catch (...) 
        { 
            LogPrintf("CDBEnv::RemoveDb: cannot remove database %s with reason of uncatched exception", name);
            if (db && 0 == db->Close()) {
                delete db;
                mapDb.erase(name);
            }
            return false;
        }
    }

    int status = -1;
    if (db)
    {
        LOCK(cs_db);
        if (mapFileUseCount[name] == 1)
        {
            CDBTransact* transaction = TxnBegin(false);
            --mapFileUseCount[name];
            status = mdb_drop(transaction->get(), db->db_, 1);
            transaction->release();
            if (status == 0)
            { 
                delete db;
                mapDb.erase(name);
                return true;
            }
            LogPrintf("CDBEnv::RemoveDb: cannot remove database %s, error %d", name, status);
        }
        else LogPrintf("CDBEnv::RemoveDb: cannot remove database %s with reason it already shared in another thread", name);
    }
    else LogPrintf("CDBEnv::RemoveDb: cannot remove database %s with reason of internal error", name);
        
    if (db && 0 == db->Close()) {
        delete db;
        mapDb.erase(name);
    }
    return false;
}


void CDBEnv::Flush(bool fShutdown)
{
    int64_t nStart = GetTimeMillis();
    // Flush log data to the actual data file on all files that are not in use
    LogPrint("db", "CDBEnv::Flush: Flush(%s)%s\n", fShutdown ? "true" : "false", fDbEnvInit ? "" : " database not started");
    if (!fDbEnvInit)
        return;
    {
        LOCK(cs_db);
        map<string, int>::iterator mi = mapFileUseCount.begin();
        while (mi != mapFileUseCount.end()) {
            string name = (*mi).first;
            auto dbIt = mapDb.find(name);
            if (dbIt == mapDb.end()) continue;

            int nRefCount = (*mi).second;
            LogPrint("db", "CDBEnv::Flush: Flushing %s (refcount = %d)...\n", name, nRefCount);
            if (nRefCount > 0) 
            {
                dbIt->second->fFlushOnClose = true;
                if (0 == dbIt->second->Close())
                {
                    delete dbIt->second;
                    mapDb.erase(name);
                }
                LogPrint("db", "CDBEnv::Flush: %s closed\n", name);
                mapFileUseCount.erase(mi++);
            } 
            else mi++;
        }

        LogPrint("db", "CDBEnv::Flush: Flush(%s)%s took %15dms\n", fShutdown ? "true" : "false", fDbEnvInit ? "" : " database not started", GetTimeMillis() - nStart);
        if (fShutdown) 
            Close();
    }
}
