// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __sig_chainparams_base_h__
#define __sig_chainparams_base_h__

#include "chain.h"

#include <string>
#include <vector>

/**
 * CBaseChainParams defines the base parameters (shared between sigecoin-cli and sigecoind)
 * of a given instance of the Sigecoin system.
 */
std::string NetworkType2String(NetworkType networkType);
NetworkType String2NetworkType(const std::string& networkTypeName);

class CBaseChainParams
{
public:
    /** BIP70 chain name strings (main, test or regtest) */
    // static const std::string MAIN;
    // static const std::string TESTNET;
    // static const std::string REGTEST;

    const std::string& DataDir() const { return strDataDir; }
    int RPCPort() const { return nRPCPort; }

protected:
    CBaseChainParams() {}

    int nRPCPort;
    std::string strDataDir;
};

/**
 * Append the help messages for the chainparams options to the
 * parameter string.
 */
void AppendParamsHelpMessages(std::string& strUsage, bool debugHelp=true);

/**
 * Return the currently selected parameters. This won't change after app
 * startup, except for unit tests.
 */
const CBaseChainParams& BaseParams();

CBaseChainParams& BaseParams(NetworkType chain);

/** Sets the params returned by Params() to those for the given network. */
void SelectBaseParams(NetworkType chain);

/**
 * Looks for -regtest, -testnet and returns the appropriate BIP70 chain name.
 * @return CBaseChainParams::MAX_NETWORK_TYPES if an invalid combination is given. CBaseChainParams::MAIN by default.
 */
NetworkType ChainNameFromCommandLine();

/**
 * Return true if SelectBaseParamsFromCommandLine() has been called to select
 * a network.
 */
bool AreBaseParamsConfigured();

#endif  /* __sig_chainparams_base_h__ */
