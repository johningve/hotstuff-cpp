#include <arpa/inet.h> // htonl
#include <asio/buffer.hpp>
#include <asio/connect.hpp>
#include <asio/read.hpp>
#include <optional>
#include <spdlog/spdlog.h>

#include "network.h"

const size_t MAX_MESSAGE_SIZE = 1024 * 1024; // 1MiB

namespace HotStuff
{

Vote::Vote()
{
}

Vote::Vote(Signature signature, Hash block_hash) : m_signature(signature), m_block_hash(block_hash)
{
}

Signature Vote::signature()
{
	return m_signature;
}

Hash Vote::block_hash()
{
	return m_block_hash;
}

Timeout::Timeout()
{
}

Timeout::Timeout(Signature signature, Round round) : m_signature(signature), m_round(round)
{
}

Signature Timeout::signature()
{
	return m_signature;
}

Round Timeout::round()
{
	return m_round;
}

class Network::Header
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

class Network::Sender : public std::enable_shared_from_this<Network::Sender>
{
  public:
	Sender(asio::ip::tcp::socket &&socket, std::shared_ptr<Network> network)
	    : m_socket(std::move(socket)), m_network(network)
	{
	}

  private:
	std::shared_ptr<Network> m_network;
	asio::ip::tcp::socket m_socket;
};

class Network::Receiver : public std::enable_shared_from_this<Network::Receiver>
{
  public:
	Receiver(asio::ip::tcp::socket &&socket, std::shared_ptr<Network> network)
	    : m_socket(std::move(socket)), m_network(network)
	{
	}

	void start()
	{
		recv_header();
	}

  private:
	std::shared_ptr<Network> m_network;
	asio::ip::tcp::socket m_socket;

	// storage for reading header / body
	Network::Header m_header;
	std::optional<std::vector<uint8_t>> m_body;

	void recv_header()
	{
		asio::async_read(m_socket, asio::buffer(&m_header, sizeof(m_header)), sizeof(Network::Header),
		                 [self = shared_from_this()](std::error_code error, size_t bytes_transferred) {
			                 if (error)
			                 {
				                 self->handle_recv_error(error);
				                 return;
			                 }
			                 auto header = self->m_header;
			                 // Convert endianness of message length.
			                 // no need to convert message type, for it is a single byte.
			                 header.size = ntohl(header.size);
			                 self->recv_body(header);
		                 });
	}

	void recv_body(Network::Header header)
	{
		if (header.size > MAX_MESSAGE_SIZE)
		{
			spdlog::debug("error reading from {1}: message size {0} exceeds limit", header.size,
			              m_socket.remote_endpoint().address().to_string());
			m_socket.close();
			return;
		}

		m_body.emplace();
		asio::async_read(m_socket, asio::dynamic_buffer(*m_body, MAX_MESSAGE_SIZE), (size_t)header.size,
		                 [self = shared_from_this(), header](std::error_code error, size_t bytes_transferred) {
			                 if (error)
			                 {
				                 self->handle_recv_error(error);
				                 return;
			                 }
			                 self->m_network->handle_message(header, std::move(*self->m_body));
			                 self->recv_header();
		                 });
	}

	void handle_recv_error(std::error_code error)
	{
		spdlog::debug("error {0} reading from {2}: {1}", error.value(), error.message(),
		              m_socket.remote_endpoint().address().to_string());
		m_socket.close();
	}
};

class Network::Server : std::enable_shared_from_this<Network::Server>
{
  public:
	Server(std::shared_ptr<Network> network, asio::io_context &io_context, uint16_t port)
	    : m_network(network), m_io_context(io_context),
	      m_acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
	{
	}

	void async_accept()
	{
		socket.emplace(m_io_context);

		m_acceptor.async_accept(*socket, [self = shared_from_this()](std::error_code error) {
			std::make_shared<Network::Receiver>(std::move(*self->socket), self->m_network)->start();
			self->async_accept();
		});
	}

  private:
	std::shared_ptr<Network> m_network;
	asio::io_context &m_io_context;
	asio::ip::tcp::acceptor m_acceptor;
	std::optional<asio::ip::tcp::socket> socket;
};

Network::Network(asio::io_context &io_context) : m_io_context(io_context), m_resolver(io_context)
{
}

void Network::connect(ID id, std::string address)
{
	asio::ip::tcp::socket socket(m_io_context);
	auto endpoint_iter = m_resolver.resolve(address);
	asio::async_connect(socket, endpoint_iter, [&, self = shared_from_this()](std::error_code error) {
		self->m_senders.insert({id, Sender(std::move(socket), self)});
	});
}

} // namespace HotStuff
