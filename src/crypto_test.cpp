#include <botan/ecdsa.h>
#include <catch2/catch_test_macros.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>

#include "blockchain.h"
#include "crypto.h"
#include "tests/util.h"

using namespace HotStuff;

TEST_CASE("Create and verify Signature", "[crypto]")
{
	auto [peers, keys] = make_peers();
	auto key = keys.find(1);
	REQUIRE(key != keys.end());
	Crypto crypto(1, key->second, peers);

	auto signature = crypto.sign(GENESIS.hash());

	REQUIRE(crypto.verify(signature, GENESIS.hash()).ok());
}

TEST_CASE("Check that valid QuorumCert is verified", "[crypto]")
{
	auto [peers, keys] = make_peers();
	auto key = keys.find(1);
	REQUIRE(key != keys.end());
	Crypto crypto(1, key->second, peers);
	std::vector<ID> signers = {2, 3, 4};
	QuorumCert qc = make_qc(peers, keys, signers);

	auto result = crypto.verify(qc, 3);
	REQUIRE(result.ok());
}

TEST_CASE("Check that incomplete QuorumCert is rejected", "[crypto]")
{
	auto [peers, keys] = make_peers();
	auto key = keys.find(1);
	REQUIRE(key != keys.end());
	Crypto crypto(1, key->second, peers);
	std::vector<ID> signers = {2, 3};

	QuorumCert qc = make_qc(peers, keys, signers);

	auto result = crypto.verify(qc, 3);
	REQUIRE(!result.ok());
	REQUIRE(result.kind() == Crypto::VerifyResult::NOT_A_QUORUM);
}

TEST_CASE("Serialize/Deserialize QuorumCert", "[crypto,serialization]")
{
	std::stringstream ss;

	auto [peers, keys] = make_peers();
	std::vector<ID> signers = {2, 3, 4};

	QuorumCert qc;

	{
		QuorumCert qc = make_qc(peers, keys, signers, GENESIS.hash(), 1);
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(qc);
	}

	{
		cereal::BinaryInputArchive iarchive(ss);
		iarchive(qc);
	}

	REQUIRE(qc.round() == 1);
}
