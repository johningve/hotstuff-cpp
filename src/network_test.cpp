#include <asio/io_context.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>

#include "network.h"

TEST_CASE("Get Server port", "[network]")
{
	asio::io_context io_context;

	auto net = std::make_shared<HotStuff::Network>(io_context);
	net->serve();

	REQUIRE(net->server_port() != 0);

	net->close();
}
