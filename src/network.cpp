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

Network::Sender::Sender(asio::ip::tcp::socket &&socket, std::shared_ptr<Network> network)
    : m_socket(std::move(socket)), m_network(network)
{
}

void Network::Sender::send_message(Network::Header header, std::vector<uint8_t> body)
{
	m_socket.async_send(asio::buffer(&header, sizeof(header)),
	                    [self = shared_from_this(), body = std::move(body)](std::error_code error, size_t _) {
		                    if (error)
		                    {
			                    spdlog::debug("error {0} sending header to {2}: {1}", error.value(), error.message(),
			                                  self->m_socket.remote_endpoint().address().to_string());
			                    self->m_socket.close();
			                    return;
		                    }

		                    self->send_body(std::move(body));
	                    });
}

void Network::Sender::send_body(std::vector<uint8_t> body)
{
	m_socket.async_send(asio::buffer(body), [self = shared_from_this()](std::error_code error, size_t _) {
		if (error)
		{
			spdlog::debug("error {0} sending body to {2}: {1}", error.value(), error.message(),
			              self->m_socket.remote_endpoint().address().to_string());
			self->m_socket.close();
			return;
		}
	});
}

Network::Receiver::Receiver(asio::ip::tcp::socket &&socket, std::shared_ptr<Network> network)
    : m_socket(std::move(socket)), m_network(network)
{
}

void Network::Receiver::start()
{
	recv_header();
}

void Network::Receiver::recv_header()
{
	asio::async_read(m_socket, asio::buffer(&m_header, sizeof(m_header)),
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

void Network::Receiver::recv_body(Network::Header header)
{
	if (header.size > MAX_MESSAGE_SIZE)
	{
		spdlog::debug("error reading from {1}: message size {0} exceeds limit", header.size,
		              m_socket.remote_endpoint().address().to_string());
		m_socket.close();
		return;
	}

	m_body.emplace();
	asio::async_read(m_socket, asio::dynamic_buffer(*m_body, header.size),
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

void Network::Receiver::handle_recv_error(std::error_code error)
{
	spdlog::debug("error {0} reading from {2}: {1}", error.value(), error.message(),
	              m_socket.remote_endpoint().address().to_string());
	m_socket.close();
}

Network::Server::Server(std::shared_ptr<Network> network, asio::io_context &io_context, uint16_t port)
    : m_network(network), m_io_context(io_context),
      m_acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
}

void Network::Server::async_accept()
{
	socket.emplace(m_io_context);

	m_acceptor.async_accept(*socket, [self = shared_from_this()](std::error_code error) {
		std::make_shared<Network::Receiver>(std::move(*self->socket), self->m_network)->start();
		self->async_accept();
	});
}

Network::Network(asio::io_context &io_context) : m_io_context(io_context), m_resolver(io_context)
{
}

void Network::connect(ID id, std::string address)
{
	asio::ip::tcp::socket socket(m_io_context);
	auto endpoint_iter = m_resolver.resolve(address);
	asio::async_connect(socket, endpoint_iter, [&, self = shared_from_this()](std::error_code error, auto _) {
		self->m_senders.insert({id, Sender(std::move(socket), self)});
	});
}

} // namespace HotStuff
