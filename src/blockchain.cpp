#include <botan/sha2_32.h>
#include <cereal/archives/binary.hpp>
#include <sstream>

#include "blockchain.h"

namespace HotStuff
{

Block::Block(Hash parent, Round round, ID proposer, QuorumCert cert)
    : m_parent(parent), m_round(round), m_proposer(proposer), m_cert(cert)
{
}

Hash Block::parent() const
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

BlockChain::BlockChain()
{
	blocks.insert({genesis.hash(), genesis});
}

std::optional<Block> BlockChain::get(Hash hash)
{
	auto block = blocks.find(hash);
	if (block == blocks.end())
	{
		return std::nullopt;
	}
	return block->second;
}

void BlockChain::add(Block block)
{
	blocks.insert({block.hash(), block});
}

} // namespace HotStuff
