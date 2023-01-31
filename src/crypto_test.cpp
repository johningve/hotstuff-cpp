#include <catch2/catch_test_macros.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>

#include "crypto.h"

using namespace HotStuff;

TEST_CASE("Serialize/Deserialize QuorumCert")
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
