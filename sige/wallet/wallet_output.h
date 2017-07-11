// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __wallet_output_h__
#define __wallet_output_h__

#include <string>

class CWalletTx;
class COutput
{
public:
    const CWalletTx *tx;
    int i;
    int nDepth;
    bool fSpendable;
    bool fSolvable;

    COutput(const CWalletTx *txIn, int iIn, int nDepthIn, bool fSpendableIn, bool fSolvableIn)
    {
        tx = txIn; i = iIn; nDepth = nDepthIn; fSpendable = fSpendableIn; fSolvable = fSolvableIn;
    }

    std::string ToString() const;
};

#endif  /* __wallet_output_h__ */
