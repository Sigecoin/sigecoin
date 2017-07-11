// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __script_script_id_h__
#define __script_script_id_h__

#include "base58.h"
#include "uint256.h"

class CScript;

/** A reference to a CScript: the Hash160 of its serialization (see script.h) */
class CScriptID : public uint160 {
public:
    CScriptID() : uint160() {}
    explicit CScriptID(const CScript& in);
    CScriptID(const uint160& in) : uint160(in) {}
public:
    base58string GetBase58addressWithNetworkScriptPrefix() const;
};

#endif  /* __script_script_id_h__ */
