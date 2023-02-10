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
	if (auto result = m_crypto.verify(block.cert()))
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

	auto opt_block_from_qc = m_blockchain.get(block.cert().block_hash());
	if (opt_block_from_qc.has_value())
	{
		auto block_from_qc = opt_block_from_qc.value();
		if (block_from_qc.round() > m_locked.round())
		{
			safe = true;
		}
	}
	else
	{
		auto opt_ancestor = m_blockchain.get(block.parent_hash());
		for (; opt_ancestor.has_value() && opt_ancestor.value().round() > m_locked.round();
		     opt_ancestor = m_blockchain.get(opt_ancestor.value().parent_hash()))
			;

		safe = opt_ancestor.has_value() && opt_ancestor.value().hash() == m_locked.hash();
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
