// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __sig_address_h__
#define __sig_address_h__

#include "base58.h"
#include "key.h"
#include "chainparams.h"

class CTxDestination;

// Blake 2 base58-encoded SIG addresses

class CSigAddress 
{
public:
    // bool Set(const CKeyID &id);
    bool IsValid() const;
    bool IsValid(const CChainParams &params) const;

    CSigAddress() {}
    CSigAddress(const base58string& strAddress) { data_.SetBase58string(strAddress); }

    CTxDestination Get() const;
    bool GetKeyID(CKeyID &keyID) const;
    bool IsScript() const;

    bool operator <(const CSigAddress& rhs) const { return this->data_ <  rhs.data_; }
    bool operator ==(const CSigAddress& rhs) const { return this->data_ == rhs.data_; }

    bool SetBase58string(const base58string& str) { return data_.SetBase58string(str) && IsValid(); }

public:
    friend class CTxDestination;

private:
    CBase58Data data_;
};

#endif  /* __sig_address_h__ */
