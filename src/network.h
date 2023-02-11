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

class Network : public std::enable_shared_from_this<Network>
{
  public:
	Network(asio::io_context &io_context);
	void serve(uint16_t port = 0);

	void connect_to(ID id, std::string address);
	uint16_t server_port();
	void close();

	void send_vote(ID recipient, Vote vote);
	void send_timeout(ID recipient, Timeout timeout);
	void broadcast_proposal(Block proposal);

  private:
	class Header
	{
	  public:
		enum class Type : uint8_t
		{
			VOTE,
			PROPOSAL,
			TIMEOUT
		};

		Type type;
		uint32_t size;
	};

	class Sender : public std::enable_shared_from_this<Network::Sender>
	{
	  public:
		Sender(asio::ip::tcp::socket &&socket, std::shared_ptr<Network> network);
		void send_message(Header header, std::vector<uint8_t> body);
		void close();

	  private:
		std::shared_ptr<Network> m_network;
		asio::ip::tcp::socket m_socket;

		void send_body(std::vector<uint8_t> body);
	};

	class Receiver : public std::enable_shared_from_this<Network::Receiver>
	{
	  public:
		Receiver(asio::ip::tcp::socket &&socket, std::shared_ptr<Network> network);
		void start();
		void close();

	  private:
		std::shared_ptr<Network> m_network;
		asio::ip::tcp::socket m_socket;

		// storage for reading header / body
		Network::Header m_header;
		std::optional<std::vector<uint8_t>> m_body;

		void recv_header();
		void recv_body(Header header);
		void handle_recv_error(std::error_code error);
	};

	class Server : public std::enable_shared_from_this<Network::Server>
	{
	  public:
		Server(std::shared_ptr<Network> network, asio::io_context &io_context, uint16_t port);
		void async_accept();
		uint16_t port();
		void close();

	  private:
		std::shared_ptr<Network> m_network;
		asio::io_context &m_io_context;
		asio::ip::tcp::acceptor m_acceptor;
		std::optional<asio::ip::tcp::socket> m_socket;
	};

	friend class Receiver;

	asio::io_context &m_io_context;
	asio::ip::tcp::resolver m_resolver;
	std::optional<Server> m_server;

	std::unordered_map<ID, std::shared_ptr<Sender>> m_senders;
	std::vector<std::shared_ptr<Receiver>> m_receivers;

	void handle_message(Header header, std::vector<uint8_t> body);
};

} // namespace HotStuff
