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
        consensus.nSubsidyHalvingInterval = 840000;
        consensus.BIP34Height = 710000;
        consensus.BIP34Hash = uint256S("fa09d204a83a768ed5a7c8d441fa62f2043abf420cff1226c7b4329aeb9d51cf");
        consensus.BIP65Height = 918684; // bab3041e8977e0dc3eeff63fe707b92bde1dd449d8efafb248c27c8264cc311a
        consensus.BIP66Height = 811879; // 7aceee012833fa8952f8835d8b1b3ae233cd6ab08fdb27a771d2bd7bdc491894
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 6048; // 75% of 8064
        consensus.nMinerConfirmationWindow = 8064; // nPowTargetTimespan / nPowTargetSpacing * 4
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1493624146; // May 1, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1498808380; // Jun 30, 2017

                                                                                       // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1498894840; // Jul 1st, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1504197832; // Aug 31st, 2017

                                                                                 // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1482163818; // Dec 19th, 2016.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1501431220; // Jul 30th, 2017.

                                                                                    // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000006805c7318ce2736c0");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x1673fa904a93848eca83d5ca82c7af974511a7e640e22edc2976420744f2e56a"); //1155631

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

        genesis = CChainParamsUtil::CreateGenesisBlock(1504197832, 2084524493, 0x1e0ffff0, 1, 5 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        std::string h1 = consensus.hashGenesisBlock.ToHexString();
        std::string h2 = genesis.hashMerkleRoot.ToHexString();

        assert(consensus.hashGenesisBlock == uint256S("0x002f1115a15c70adf4e3b728d06b44607c4fb6cb4fd36f9754ec271c9d93d618"));
        assert(genesis.hashMerkleRoot == uint256S("0x0b58bf0eac506e7bcf626a900c8e44ed3ea4ce7b34f96acd8d2ff9bb87e5a94e"));

        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.push_back(CDNSSeedData("localhost", "127.0.0.1", false));     // loopback

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 63);  // {63}
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 37);  // {37}
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 1);       // {1}
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >(); // {0x04,0x88,0xB2,0x1E}
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >(); // {0x04,0x88,0xAD,0xE4}

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = true;

        MapCheckpoints tmp = boost::assign::map_list_of
        (0, uint256S("0x002f1115a15c70adf4e3b728d06b44607c4fb6cb4fd36f9754ec271c9d93d618"));
        checkpointData = CCheckpointData(tmp);

        chainTxData = ChainTxData{
            // Data as of block 00000000000000000166d612d5595e2b1cd88d71d695fc580af64d8da8658c23 (height 446482).
            0, // * UNIX timestamp of last known number of transactions
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

