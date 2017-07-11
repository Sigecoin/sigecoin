// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __sig_keybase_h__
#define __sig_keybase_h__

#include "base58.h"
#include "extkey.h"
#include "extpubkey.h"
#include "chainparams.h"

template<typename K, int Size, CChainParams::Base58Type Type> class CSigecoinExtKeyBase : public CBase58Data
{
public:
    void SetKey(const K &key) {
        unsigned char vch[Size];
        key.Encode(vch);
        SetData(Params().Base58Prefix(Type), vch, vch + Size);
    }

    K GetKey() {
        K ret;
        if (m_vchData.size() == Size) {
            // If base58 encoded data does not hold an ext key, return a !IsValid() key
            ret.Decode(&m_vchData[0]);
        }
        return ret;
    }

    CSigecoinExtKeyBase(const K &key) {
        SetKey(key);
    }

    CSigecoinExtKeyBase(const std::string& strBase58c) {
        this->SetBase58string(base58string(strBase58c, Params().Base58Prefix(Type).size()));
    }

    CSigecoinExtKeyBase() {}
};

typedef CSigecoinExtKeyBase<CExtKey, BIP32_EXTKEY_SIZE, CChainParams::EXT_SECRET_KEY> CSigecoinExtKey;
typedef CSigecoinExtKeyBase<CExtPubKey, BIP32_EXTKEY_SIZE, CChainParams::EXT_PUBLIC_KEY> CSigecoinExtPubKey;

#endif  /* __sig_keybase_h__ */
