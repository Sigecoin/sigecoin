// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include <assert.h>
#include <boost/assign/list_of.hpp>
#include "ChainParamsUtil.h"
#include "chainparamsseeds.h"
#include "utilstrencodings.h"

/**
* Main network
*/
/**
* What makes a good checkpoint block?
* + Is surrounded by blocks with reasonable timestamps
*   (no blocks before with a timestamp after, none after with
*    timestamp before)
* + Contains no strange transactions
*/

class CMainParams : public CChainParams {
public:
    CMainParams()
    : CChainParams(NETWORK_MAIN)
    {
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.BIP65Height = 1;  // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.BIP66Height = 1;  // 00000000000000000379eaa19dce8c9b722d46ae6a57c2f1a988119488b50931
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 3 * 24 * 60 * 60; // three days
        consensus.nPowTargetSpacing = 2 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1890; // 90% of 2160
        consensus.nMinerConfirmationWindow = 2160; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1493624146; // May 1, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1498808380; // Jun 30, 2017

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1498894840; // Jul 1st, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1501573267; // Aug 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1482163818; // Dec 19th, 2016.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1501431220; // Jul 30th, 2017.

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000006805c7318ce2736c0"); // ???

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000030abc968e1bd635736e880b946085c93152969b9a81a6e2"); // ??? (447235)

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
        */
        pchMessageStart[0] = 0xf7;
        pchMessageStart[1] = 0xbc;
        pchMessageStart[2] = 0xb2;
        pchMessageStart[3] = 0xd7;
        nDefaultPort = 8560;
        nPruneAfterHeight = 100000;

        genesis = CChainParamsUtil::CreateGenesisBlock(1501573267, 2083236893, 0x1c00ffff, 1, 5 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        std::string h1 = consensus.hashGenesisBlock.ToHexString();
        std::string h2 = genesis.hashMerkleRoot.ToHexString();

        assert(consensus.hashGenesisBlock == uint256S("0x6af3fc15eaf476d088f8c689f0b4289e8b11ab509891e0e4510f448dc9d5c0ad"));
        assert(genesis.hashMerkleRoot == uint256S("0x63f1cdf1b5d523653bac0fa1e1b61ac93a625fe9e0f2cab722164fe403de096c"));

        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.push_back(CDNSSeedData("localhost", "seed.localhost", true)); // localhost

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 63);  // {63}
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 37);  // {37}
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 1);       // {1}
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >(); // {0x04,0x88,0xB2,0x1E}
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >(); // {0x04,0x88,0xAD,0xE4}

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        MapCheckpoints tmp = boost::assign::map_list_of
        (0, uint256S("0xe2f277e45189eda8a0aa2f449085bdb04eb6feca1a02f6d9878854303d0954fa"));
        checkpointData = CCheckpointData(tmp);

        chainTxData = ChainTxData{
            // Data as of block 00000000000000000166d612d5595e2b1cd88d71d695fc580af64d8da8658c23 (height 446482).
            1506843944, // * UNIX timestamp of last known number of transactions
            0,          // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0           // * estimated number of transactions per second after that timestamp
        };
    }
};
CMainParams* g_mainParams;

void Init1() {
    g_mainParams = new CMainParams();
}

