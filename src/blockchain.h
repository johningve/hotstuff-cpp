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
	// Returns an empty Block.
	// You probably shouldn't use this unless you need it for deserialization.
	Block();
	Block(Hash parent, Round round, ID proposer, QuorumCert cert, std::vector<uint8_t> payload = {});

	Hash parent_hash() const;
	Round round() const;
	ID proposer() const;
	QuorumCert cert() const;
	Hash hash() const;
	std::vector<uint8_t> payload() const;

  private:
	friend class cereal::access;

	Hash m_parent;
	Round m_round;
	ID m_proposer;
	QuorumCert m_cert;
	std::vector<uint8_t> m_payload;

	template <class Archive> void serialize(Archive &archive)
	{
		archive(m_parent, m_round, m_proposer, m_cert);
	}
};

const Block GENESIS(Hash(), 0, 0, GENESIS_QC);

class BlockChain
{
	std::unordered_map<Hash, Block> blocks;

  public:
	BlockChain();
	std::optional<Block> get(Hash hash);
	void add(Block block);
};

} // namespace HotStuff
