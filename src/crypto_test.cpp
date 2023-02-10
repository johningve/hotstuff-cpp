#include <botan/ecdsa.h>
#include <catch2/catch_test_macros.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>

#include "blockchain.h"
#include "crypto.h"

using namespace HotStuff;

Botan::ECDSA_PrivateKey gen_key()
{
	Botan::ECDSA_PrivateKey key(Botan::system_rng(), Botan::EC_Group("secp256k1"));
	return key;
}

std::pair<std::shared_ptr<Peers>, std::unordered_map<ID, Botan::ECDSA_PrivateKey>> make_peers(int num_peers = 4,
                                                                                              ID first_id = 1)
{
	std::unordered_map<ID, Botan::ECDSA_PrivateKey> keys;
	auto peers = std::make_shared<Peers>();

	for (ID i = first_id; i < first_id + num_peers; i++)
	{
		auto key = gen_key();
		keys.insert({i, key});
		peers->add({i, key});
	}

	return {peers, keys};
}

QuorumCert make_qc(std::shared_ptr<Peers> peers, const std::unordered_map<ID, Botan::ECDSA_PrivateKey> &keys,
                   std::vector<ID> &signers, Hash hash = GENESIS.hash(), Round round = 1)
{
	std::vector<Signature> sigs;
	for (ID i : signers)
	{
		auto key = keys.find(i);
		REQUIRE(key != keys.end());

		Crypto crypto(i, key->second, peers);

		sigs.push_back(crypto.sign(hash));
	}

	return QuorumCert(hash, round, std::move(sigs));
}

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

	QuorumCert qc = make_qc(peers, keys, signers);

	{
		QuorumCert qc(Hash(), 10, std::vector<Signature>());
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(qc);
	}

	{
		cereal::BinaryInputArchive iarchive(ss);
		iarchive(qc);
	}

	REQUIRE(qc.round() == 10);
}
