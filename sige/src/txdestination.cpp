// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdestination.h"
#include "sigaddress.h"

namespace
{
    class CTxDestinationVisitor : public boost::static_visitor<base58string>
    {
    public:
        base58string operator()(const CKeyID& id) const { return id.GetBase58addressWithNetworkPubkeyPrefix(); }
        base58string operator()(const CScriptID& id) const { return id.GetBase58addressWithNetworkScriptPrefix(); }
        base58string operator()(const CNoDestination& no) const { return base58string(); }
    };

}

base58string CTxDestination::GetBase58addressWithNetworkPrefix() const
{
    return boost::apply_visitor(CTxDestinationVisitor(), *this);
}

bool CTxDestination::IsNoDestination() const
{
    return this->type() == typeid(CNoDestination);
}
