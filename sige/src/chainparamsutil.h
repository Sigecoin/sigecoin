// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __sig_chainparams_util_h__
#define __sig_chainparams_util_h__

#include "amount.h"
#include "chain.h"

#include <map>

class CChainParams;
class CBlock;
class CScript;

class CChainParamsUtil {
private:
    static CBlock _CreateGenesisBlockImpl(
        const char* pszTimestamp,
        const CScript& genesisOutputScript,
        uint32_t nTime,
        uint32_t nNonce,
        uint32_t nBits,
        int32_t nVersion,
        const CAmount& genesisReward
    );
public:
    static CBlock CreateGenesisBlock(
        uint32_t nTime,
        uint32_t nNonce,
        uint32_t nBits,
        int32_t nVersion,
        const CAmount& genesisReward
    );
    static CChainParams* GetParamsOfNetworkType(NetworkType networkType) {
        return m_paramsOfType[networkType];
    }
    static void RegisterParamsOfNetworkType(NetworkType networkType, CChainParams* pParams) {
        m_paramsOfType[networkType] = pParams;
    }
private:
    static std::map<NetworkType, CChainParams*> m_paramsOfType;
};

#endif  /* __sig_chainparams_util_h__ */
