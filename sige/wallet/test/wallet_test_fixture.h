// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __wallet_test_fixture_h__
#define __wallet_test_fixture_h__

#include "test/test_sigecoin.h"

/** Testing setup and teardown for wallet.
 */
struct WalletTestingSetup: public TestingSetup {
    WalletTestingSetup(NetworkType chainType = NETWORK_MAIN);
    ~WalletTestingSetup();
};

#endif  /* __wallet_test_fixture_h__ */
