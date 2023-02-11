#include <asio/io_context.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <fmt/core.h>
#include <thread>

#include "blockchain.h"
#include "crypto.h"
#include "network.h"
#include "tests/util.h"

using namespace std::chrono_literals;

TEST_CASE("Get Server port", "[network]")
{
	asio::io_context io_context;

	auto net = std::make_shared<HotStuff::Network>(io_context);
	net->serve();

	REQUIRE(net->server_port() != 0);

	net->close();
}

TEST_CASE("Send vote", "[network]")
{
	asio::io_context io_context;

	HotStuff::Crypto crypto(1, gen_key(), std::make_shared<Peers>());
	auto sig = crypto.sign(GENESIS.hash());
	HotStuff::Vote vote(sig, GENESIS.hash());

	auto net1 = std::make_shared<HotStuff::Network>(io_context);
	auto net2 = std::make_shared<HotStuff::Network>(io_context);

	net1->serve(0, [&]() {
		net2->connect_to(1, fmt::format("localhost:{}", net1->server_port()), [&]() { net2->send_vote(1, vote); });
	});

	// net2->serve();

	bool cb_fired = false;

	net1->on_vote([&](HotStuff::Vote vote) {
		REQUIRE(vote.block_hash() == GENESIS.hash());
		REQUIRE(crypto.verify(vote.signature(), vote.block_hash()).ok());
		cb_fired = true;
		io_context.stop();
	});

	auto thread = std::thread([&]() { REQUIRE_NOTHROW(io_context.run_for(1s)); });

	thread.join();

	REQUIRE(cb_fired);
}
