// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include <assert.h>
#include <boost/assign/list_of.hpp>
#include "ChainParamsUtil.h"
#include "chainparamsseeds.h"
#include "utilstrencodings.h"

/**
* Testnet (v3)
*/
class CTestNetParams : public CChainParams {
public:
    CTestNetParams()
    : CChainParams(NETWORK_TESTNET)
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
        consensus.nRuleChangeActivationThreshold = 1260; // 60% of 2160
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

        pchMessageStart[0] = 0x09;
        pchMessageStart[1] = 0x0f;
        pchMessageStart[2] = 0x07;
        pchMessageStart[3] = 0x05;
        nDefaultPort = 18560;
        nPruneAfterHeight = 1000;

        genesis = CChainParamsUtil::CreateGenesisBlock(1501573267, 414098458, 0x1c0ffff0, 1, 5 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        std::string h1 = consensus.hashGenesisBlock.ToHexString();
        std::string h2 = genesis.hashMerkleRoot.ToHexString();

        assert(consensus.hashGenesisBlock == uint256S("0x0c3baaee84ce6eed273b83fda7adb5f7481ae1d96acb25832e90478678ecce8b"));
        assert(genesis.hashMerkleRoot == uint256S("0x0b58bf0eac506e7bcf626a900c8e44ed3ea4ce7b34f96acd8d2ff9bb87e5a94e"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.push_back(CDNSSeedData("localhost", "seed.localhost", true)); // localhost

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 63);  // {63}
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 37);  // {37}
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 1);       // {1}
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >(); // {0x04,0x35,0x87,0xCF}
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >(); // {0x04,0x35,0x83,0x94}

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        MapCheckpoints tmp = boost::assign::map_list_of
        (0, uint256S("c08af7be0de8a97a5ee09979c02c5122dc1d077e243600ccd189f19c545722c4"));
        checkpointData = CCheckpointData(tmp);

        chainTxData = ChainTxData{
            // Data as of block 00000000c2872f8f8a8935c8e3c5862be9038c97d4de2cf37ed496991166928a (height 1063660)
            1506843944,
            0,
            0
        };
    }
};
CTestNetParams* g_testNetParams;

void Init2() {
    g_testNetParams = new CTestNetParams();
}
