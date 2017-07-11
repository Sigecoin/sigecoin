// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __script_standard_h__
#define __script_standard_h__

#include <boost/variant.hpp>
#include <stdint.h>

#include "uint256.h"
#include "pubkey.h" // CKeyID
#include "script/script.h"
#include "txdestination.h"

static const bool DEFAULT_ACCEPT_DATACARRIER = true;

static const unsigned int MAX_OP_RETURN_RELAY = 83; //!< bytes (+1 for OP_RETURN, +2 for the pushdata opcodes)
extern bool fAcceptDatacarrier;
extern unsigned nMaxDatacarrierBytes;

const char* GetTxnOutputType(txnouttype t);

bool Solver(const CScript& scriptPubKey, txnouttype* typeRet, std::vector<std::vector<unsigned char> >& vSolutionsRet);
bool ExtractDestination(const CScript& scriptPubKey, CTxDestination& addressRet);
bool ExtractDestinations(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<CTxDestination>& addressRet, int& nRequiredRet);

CScript GetScriptForDestination(const CTxDestination& dest);
CScript GetScriptForRawPubKey(const CPubKey& pubkey);
CScript GetScriptForMultisig(int nRequired, const std::vector<CPubKey>& keys);
CScript GetScriptForWitness(const CScript& redeemscript);

#endif  /* __script_standard_h__ */
