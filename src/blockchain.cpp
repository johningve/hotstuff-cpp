#include "blockchain.h"

namespace HotStuff
{

Block::Block(Hash parent, Round round, ID proposer, QuorumCert cert)
    : m_parent(parent), m_round(round), m_proposer(proposer), m_cert(cert)
{
}

Hash Block::parent()
{
	return m_parent;
}

Round Block::round()
{
	return m_round;
}

ID Block::proposer()
{
	return m_proposer;
}

QuorumCert Block::cert()
{
	return m_cert;
}

Hash Block::hash()
{
	return Hash();
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
	blocks.insert(std::make_pair(block.hash(), block));
}

} // namespace HotStuff
