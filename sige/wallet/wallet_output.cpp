// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "wallet_output.h"
#include "tinyformat.h"
#include "wallet_tx.h"
#include "utilmoneystr.h"

std::string COutput::ToString() const
{
    return strprintf("COutput(%s, %d, %d) [%s]",
        tx->GetHash().ToHexString(), i, nDepth, FormatMoney(tx->tx->vout[i].nValue));
}
