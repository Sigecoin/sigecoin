// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include <assert.h>
#include <boost/assign/list_of.hpp>
#include "ChainParamsUtil.h"

/**
* Regression test
*/
class CRegTestParams : public CChainParams {
public:
    CRegTestParams()
    : CChainParams(NETWORK_REGTEST)
    {
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 3 * 24 * 60 * 60; // three days
        consensus.nPowTargetSpacing = 2 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 1890; // 90% of 2160
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2160)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xf7;
        pchMessageStart[1] = 0xbc;
        pchMessageStart[2] = 0xb2;
        pchMessageStart[3] = 0xd7;
        nDefaultPort = 8561;
        nPruneAfterHeight = 1000;

        genesis = CChainParamsUtil::CreateGenesisBlock(1501573267, 1, 0x207fffff, 1, 55 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        std::string h1 = consensus.hashGenesisBlock.ToHexString();
        std::string h2 = genesis.hashMerkleRoot.ToHexString();

        assert(consensus.hashGenesisBlock == uint256S("0xa606103d6bd5df7a33de58dd21a08019c8c283f32c62eeaae3c8445fd1e4d1ed"));
        assert(genesis.hashMerkleRoot == uint256S("0x98c41170a2d0620725cd756b78b482a74806a7b9887fe27d0fb797c1eddd6893"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        MapCheckpoints tmp = boost::assign::map_list_of
        (0, uint256S("0x6bdf88ff70c6ca84db43cfdc013ee3bd65ed5fa02490e2b0bbe00c27f842a662"));
        checkpointData = CCheckpointData(tmp);

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 63);  // {63}
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 37);  // {37}
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 1);       // {1}
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >(); // {0x04,0x35,0x87,0xCF}
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >(); // {0x04,0x35,0x83,0x94}
    }

    void UpdateBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
};
CRegTestParams* g_regTestParams;


void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    static_cast<CRegTestParams&>(Params(NETWORK_REGTEST)).UpdateBIP9Parameters(d, nStartTime, nTimeout);
}

void Init3() {
    g_regTestParams = new CRegTestParams();
}
