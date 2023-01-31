#pragma once

#include <cereal/access.hpp>
#include <optional>
#include <unordered_map>

#include "crypto.h"
#include "types.h"

namespace HotStuff
{

class Block
{
  public:
	Block(Hash parent, Round round, ID proposer, QuorumCert cert);

	Hash parent();
	Round round();
	ID proposer();
	QuorumCert cert();
	Hash hash();

  private:
	friend class cereal::access;

	Hash m_parent;
	Round m_round;
	ID m_proposer;
	QuorumCert m_cert;

	template <class Archive> void serialize(Archive &archive)
	{
		archive(m_parent, m_round, m_proposer, m_cert);
	}
};

static const Block genesis(Hash(), 0, 0, genesis_qc);

class BlockChain
{
	std::unordered_map<Hash, Block> blocks;

  public:
	std::optional<Block> get(Hash hash);
	void add(Block block);
};

} // namespace HotStuff
