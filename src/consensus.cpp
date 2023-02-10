#include <iostream>
#include <optional>

#include "consensus.h"

namespace HotStuff
{

Hash Vote::block_hash()
{
	return m_hash;
}

Signature Vote::signature()
{
	return m_sig;
}

LeaderElection::LeaderElection(int num_replicas) : m_num_replicas(num_replicas)
{
}

ID LeaderElection::get_leader(Round round)
{
	return (ID)round % m_num_replicas;
}

void Consensus::on_propose(Block block)
{
	if (auto result = m_crypto.verify(block.cert(), 3))
	{
		std::cerr << "on_propose: Invalid quorum cert." << std::endl;
		return;
	}

	if (block.proposer() != m_leader_election.get_leader(block.round()))
	{
		std::cerr << "on_propose: Block was not proposed by expected leader." << std::endl;
		return;
	}

	bool safe = false;

	auto block_from_qc = m_blockchain.get(block.cert().block_hash());
	if (block_from_qc)
	{
		if (block_from_qc->round() > m_locked.round())
		{
			safe = true;
		}
	}
	else
	{
		auto ancestor = m_blockchain.get(block.parent_hash());
		for (; ancestor && ancestor->round() > m_locked.round(); ancestor = m_blockchain.get(ancestor->parent_hash()))
			;

		safe = ancestor && ancestor->hash() == m_locked.hash();
	}

	if (!safe)
	{
		std::cerr << "on_propose: Block was rejected" << std::endl;
		return;
	}

	std::cerr << "on_propose: Block was accepted" << std::endl;

	m_blockchain.add(block);
	m_synchronizer.update(block.cert());

	if (block.round() <= m_voted)
	{
		std::cerr << "on_propose: Already voted in this view!" << std::endl;
		return;
	}

	// TODO: create vote

	auto signature = m_crypto.sign(block.hash());
}

void Consensus::on_vote()
{
}

} // namespace HotStuff
