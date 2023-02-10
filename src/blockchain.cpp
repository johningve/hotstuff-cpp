#include <botan/sha2_32.h>
#include <cereal/archives/binary.hpp>
#include <sstream>

#include "blockchain.h"

namespace HotStuff
{

Block::Block()
{
}

Block::Block(Hash parent, Round round, ID proposer, QuorumCert cert, std::vector<uint8_t> payload)
    : m_parent(parent), m_round(round), m_proposer(proposer), m_cert(cert), m_payload(payload)
{
}

Hash Block::parent_hash() const
{
	return m_parent;
}

Round Block::round() const
{
	return m_round;
}

ID Block::proposer() const
{
	return m_proposer;
}

QuorumCert Block::cert() const
{
	return m_cert;
}

Hash Block::hash() const
{
	Hash hash;
	Botan::SHA_256 hasher;
	std::stringstream buf;

	{
		cereal::BinaryOutputArchive oa(buf);
		oa(*this);
	}

	auto hash_vec = hasher.process(buf.str());
	std::copy_n(hash_vec.begin(), hash.size(), hash.begin());

	return hash;
}

std::vector<uint8_t> Block::payload() const
{
	return m_payload;
}

BlockChain::BlockChain()
{
	add(GENESIS);
}

std::optional<Block> BlockChain::get(Hash hash)
{
	auto block_entry = blocks.find(hash);
	if (block_entry == blocks.end())
	{
		return std::nullopt;
	}
	return block_entry->second;
}

void BlockChain::add(Block block)
{
	blocks.insert({block.hash(), block});
}

} // namespace HotStuff
