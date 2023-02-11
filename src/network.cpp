#include <arpa/inet.h> // htonl
#include <asio/buffer.hpp>
#include <asio/connect.hpp>
#include <asio/read.hpp>
#include <cereal/archives/binary.hpp>
#include <optional>
#include <spdlog/spdlog.h>
#include <sstream>

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

Network::Header::Header()
{
}

Network::Header::Header(Type type, uint32_t size) : type(type), size(size)
{
}

Network::Sender::Sender(asio::ip::tcp::socket &&socket, std::shared_ptr<Network> network)
    : m_socket(std::move(socket)), m_network(network)
{
}

void Network::Sender::send_message(Header header, std::vector<uint8_t> body)
{
	m_socket.async_send(asio::buffer(&header, sizeof(header)),
	                    [self = shared_from_this(), body = std::move(body)](std::error_code error, size_t _) {
		                    if (error)
		                    {
			                    spdlog::error("error {0} sending header to {2}: {1}", error.value(), error.message(),
			                                  self->m_socket.remote_endpoint().address().to_string());
			                    self->m_socket.close();
			                    return;
		                    }

		                    self->send_body(std::move(body));
	                    });
}

void Network::Sender::close()
{
	m_socket.close();
}

void Network::Sender::send_body(std::vector<uint8_t> body)
{
	m_socket.async_send(asio::buffer(body), [self = shared_from_this()](std::error_code error, size_t _) {
		if (error)
		{
			spdlog::error("error {0} sending body to {2}: {1}", error.value(), error.message(),
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

void Network::Receiver::close()
{
	m_socket.close();
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

void Network::Receiver::recv_body(Header header)
{
	if (header.size > MAX_MESSAGE_SIZE)
	{
		spdlog::error("error reading from {1}: message size {0} exceeds limit", header.size,
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
	spdlog::error("error {0} reading from {2}: {1}", error.value(), error.message(),
	              m_socket.remote_endpoint().address().to_string());
	m_socket.close();
}

Network::Server::Server(std::shared_ptr<Network> network, asio::io_context &io_context, uint16_t port)
    : m_network(network), m_io_context(io_context),
      m_acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{
}

void Network::Server::async_accept(std::function<void()> callback)
{
	m_socket.emplace(m_io_context);

	m_acceptor.async_accept(
	    *m_socket, [self = shared_from_this(), callback = std::move(callback)](std::error_code error) {
		    auto recv = std::make_shared<Network::Receiver>(std::move(*self->m_socket), self->m_network);
		    recv->start();
		    self->m_network->m_receivers.push_back(recv);
		    callback();
		    self->async_accept();
	    });
}

uint16_t Network::Server::port()
{
	return m_acceptor.local_endpoint().port();
}

void Network::Server::close()
{
	m_acceptor.close();
}

Network::Network(asio::io_context &io_context) : m_io_context(io_context), m_resolver(io_context)
{
}

void Network::serve(uint16_t port, std::function<void()> callback)
{
	m_server = std::make_shared<Server>(shared_from_this(), m_io_context, port);
	m_server->async_accept(std::move(callback));
}

void Network::connect_to(ID id, std::string address, std::function<void()> callback)
{
	asio::ip::tcp::socket socket(m_io_context);
	auto endpoint_iter = m_resolver.resolve(address);
	asio::async_connect(socket, endpoint_iter,
	                    [&, self = shared_from_this(), callback = std::move(callback)](std::error_code error, auto _) {
		                    self->m_senders.insert({id, std::make_shared<Sender>(std::move(socket), self)});
		                    callback();
	                    });
}

uint16_t Network::server_port()
{
	return m_server->port();
}

void Network::close()
{
	for (auto [_, sender] : m_senders)
	{
		sender->close();
	}

	for (auto receiver : m_receivers)
	{
		receiver->close();
	}

	m_server->close();
}

void Network::send_vote(ID recipient, Vote vote)
{
	send_message<Vote, Header::Type::VOTE>(recipient, vote);
}

void Network::send_timeout(ID recipient, Timeout timeout)
{
	send_message<Timeout, Header::Type::TIMEOUT>(recipient, timeout);
}

void Network::broadcast_proposal(Block proposal)
{
	// FIXME: this serializes the block each time it is sent.
	// Instead, it should be serialized only once and then sent.
	for (auto [id, _] : m_senders)
	{
		send_message<Block, Header::Type::PROPOSAL>(id, proposal);
	}
}

void Network::on_vote(std::function<void(Vote)> callback)
{
	m_cb_vote = callback;
}

void Network::on_timeout(std::function<void(Timeout)> callback)
{
	m_cb_timeout = callback;
}

void Network::on_propose(std::function<void(Block)> callback)
{
	m_cb_proposal = callback;
}

template <typename Message, Network::Header::Type Type> void Network::send_message(ID recipient, Message message)
{
	auto sender = m_senders.find(recipient);
	if (sender == m_senders.end())
	{
		spdlog::error("unknown recipient {}", recipient);
		return;
	}

	// FIXME: this is probably doing multiple unnecessary copies
	std::stringstream ss;
	cereal::BinaryOutputArchive oarchive(ss);

	oarchive(message);
	const std::string &tmp_str = ss.str();
	std::vector<uint8_t> body(tmp_str.begin(), tmp_str.end());

	sender->second->send_message(Header(Type, body.size()), std::move(body));
}

void Network::handle_message(Header header, std::vector<uint8_t> body)
{
	// FIXME: find a better way to deserialize the body
	std::stringstream ss(std::string(body.begin(), body.end()));

	cereal::BinaryInputArchive iarchive(ss);

	switch (header.type)
	{
	case Header::Type::VOTE: {
		Vote vote;
		iarchive(vote);
		m_cb_vote(vote);
		break;
	}
	case Header::Type::TIMEOUT: {
		Timeout timeout;
		iarchive(timeout);
		m_cb_timeout(timeout);
		break;
	}
	case Header::Type::PROPOSAL: {
		Block block;
		iarchive(block);
		m_cb_proposal(block);
		break;
	}
	default:
		spdlog::error("unknown message type");
		break;
	}
}

} // namespace HotStuff
