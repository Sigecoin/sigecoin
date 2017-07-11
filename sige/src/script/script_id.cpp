// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "script/script_id.h"
#include "script/script.h"
#include "sigaddress.h"

base58string CScriptID::GetBase58addressWithNetworkScriptPrefix() const
{
    base58string str;
    str.SetData(Params().Base58Prefix(CChainParams::SCRIPT_ADDRESS), this, 20); // ###### script の prefix
    return str;
}
