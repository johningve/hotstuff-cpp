#pragma once

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <cereal/access.hpp>
#include <unordered_map>

#include "blockchain.h"
#include "crypto.h"
#include "types.h"

namespace HotStuff
{

class Vote
{
  public:
	// Creates an empty Vote.
	// You probably shouldn't use this unless you need it for deserialization.
	Vote();
	Vote(Signature signature, Hash block_hash);

	Signature signature();
	Hash block_hash();

  private:
	friend class cereal::access;

	Signature m_signature;
	Hash m_block_hash;

	template <class Archive> void serialize(Archive &archive)
	{
		archive(m_signature, m_block_hash);
	}
};

class Timeout
{
  public:
	// Creates an empty Timeout.
	// You probably shouldn't use this unless you need it for deserialization.
	Timeout();
	Timeout(Signature signature, Round round);

	Signature signature();
	Round round();

  private:
	friend class cereal::access;

	Signature m_signature;
	Round m_round;

	template <class Archive> void serialize(Archive &archive)
	{
		archive(m_signature, m_round);
	}
};

class Network : std::enable_shared_from_this<Network>
{
  public:
	Network(asio::io_context &io_context);

	void send_vote(ID recipient, Vote vote);
	void send_timeout(ID recipient, Timeout timeout);
	void broadcast_proposal(Block proposal);

  private:
	class Server;
	class Sender;
	class Receiver;
	class Header;

	friend class Receiver;

	asio::io_context &m_io_context;
	asio::ip::tcp::resolver m_resolver;

	std::unordered_map<ID, Sender> m_senders;

	void handle_message(Header header, std::vector<uint8_t> body);
	void connect(ID id, std::string address);
};

} // namespace HotStuff
