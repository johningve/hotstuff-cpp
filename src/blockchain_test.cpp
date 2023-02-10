#include <catch2/catch_test_macros.hpp>

#include "blockchain.h"

using namespace HotStuff;

TEST_CASE("Get Genesis", "[blockchain]")
{
	BlockChain bc;

	auto block = bc.get(GENESIS.hash());

	REQUIRE(block.has_value());

	REQUIRE(block.value().hash() == GENESIS.hash());
}
