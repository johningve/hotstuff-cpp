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

TEST_CASE("Create and verify Signature", "[crypto]")
{
	Peers peers;
	Crypto crypto(1, gen_key(), peers);
	auto signature = crypto.sign(genesis.hash());

	REQUIRE(crypto.verify(signature, genesis.hash()).ok());
}

TEST_CASE("Serialize/Deserialize QuorumCert", "[crypto]")
{
	std::stringstream ss;

	QuorumCert qc = genesis_qc;

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
