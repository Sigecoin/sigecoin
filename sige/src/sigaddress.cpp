// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "sigaddress.h"
#include "txdestination.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSigAddress::IsValid() const
{
    return IsValid(Params());
}

bool CSigAddress::IsValid(const CChainParams& params) const
{
    bool fCorrectSize = data_.m_vchData.size() == 20;
    bool fKnownVersion = data_.m_vchVersion == params.Base58Prefix(CChainParams::PUBKEY_ADDRESS) ||
        data_.m_vchVersion == params.Base58Prefix(CChainParams::SCRIPT_ADDRESS);
    return fCorrectSize && fKnownVersion;
}

CTxDestination CSigAddress::Get() const
{
    if (!IsValid())
        return CNoDestination();
    uint160 id;
    memcpy(&id, &data_.m_vchData[0], 20);
    if (data_.m_vchVersion == Params().Base58Prefix(CChainParams::PUBKEY_ADDRESS))
        return CKeyID(id);
    else if (data_.m_vchVersion == Params().Base58Prefix(CChainParams::SCRIPT_ADDRESS))
        return CScriptID(id);
    else
        return CNoDestination();
}

bool CSigAddress::GetKeyID(CKeyID& keyID) const
{
    if (!IsValid() || data_.m_vchVersion != Params().Base58Prefix(CChainParams::PUBKEY_ADDRESS))
        return false;
    uint160 id;
    memcpy(&id, &data_.m_vchData[0], 20);
    keyID = CKeyID(id);
    return true;
}

bool CSigAddress::IsScript() const
{
    return IsValid() && data_.m_vchVersion == Params().Base58Prefix(CChainParams::SCRIPT_ADDRESS);
}
