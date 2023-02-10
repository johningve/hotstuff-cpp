#include <catch2/catch_test_macros.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>

#include "blockchain.h"
#include "tests/util.h"

using namespace HotStuff;

TEST_CASE("Get Genesis", "[blockchain]")
{
	BlockChain bc;

	auto block = bc.get(GENESIS.hash());

	REQUIRE(block.has_value());

	REQUIRE(block.value().hash() == GENESIS.hash());
}

TEST_CASE("Serialize/Deserialize Block", "[serialization]")
{
	auto [peers, keys] = make_peers();
	auto qc = make_qc(peers, keys);
	auto genesis_hash = GENESIS.hash();
	Block block1(genesis_hash, 1, 1, qc, {genesis_hash.begin(), genesis_hash.end()});
	Block block2 = GENESIS;

	std::stringstream ss;

	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(block1);
	}

	{
		cereal::BinaryInputArchive iarchive(ss);
		iarchive(block2);
	}

	REQUIRE(block1.hash() == block2.hash());
}
