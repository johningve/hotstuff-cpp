#pragma once

#include "blockchain.h"
#include "crypto.h"
#include "synchronizer.h"
#include "types.h"

namespace HotStuff
{

class Vote
{
  public:
	Hash block_hash();
	Signature signature();

  private:
	Hash m_hash;
	Signature m_sig;
};

class LeaderElection
{
  public:
	LeaderElection(int num_replicas);
	ID get_leader(Round round);

  private:
	int m_num_replicas;
};

class Consensus
{
  public:
	void on_propose(Block block);
	void on_vote();

  private:
	Block m_locked;
	Block m_executed;
	Round m_voted;

	std::shared_ptr<BlockChain> m_blockchain;
	std::shared_ptr<Crypto> m_crypto;
	std::shared_ptr<LeaderElection> m_leader_election;
	std::shared_ptr<Synchronizer> m_synchronizer;
};

} // namespace HotStuff
