#include <botan/sha2_32.h>
#include <fmt/core.h>

#include "crypto.h"

namespace HotStuff
{

ID Signature::signer() const
{
	return m_signer;
}

Signature::Signature()
{
}

Signature::Signature(ID signer, std::vector<uint8_t> signature) : m_signer(signer), m_signature(signature)
{
}

Hash QuorumCert::block_hash() const
{
	return m_block;
}

Round QuorumCert::round() const
{
	return m_round;
}

QuorumCert::QuorumCert(Hash block, Round round, std::vector<Signature> signatures)
    : m_block(block), m_round(round), m_signatures(signatures)
{
}

QuorumCert::QuorumCert(const QuorumCert &other) : m_block(other.m_block), m_round(other.m_round)
{
	this->m_signatures = std::vector<Signature>(other.m_signatures);
}

std::vector<ID> QuorumCert::signers() const
{
	std::vector<ID> signers;

	for (auto &signature : m_signatures)
	{
		signers.push_back(signature.signer());
	}

	return signers;
}

Round TimeoutCert::round()
{
	return m_round;
}

TimeoutCert::TimeoutCert(Round round, std::vector<Signature> signatures) : m_round(round), m_signatures(signatures)
{
}

TimeoutCert::TimeoutCert(const TimeoutCert &other) : m_round(other.m_round)
{
	this->m_signatures = std::vector<Signature>(other.m_signatures);
}

std::vector<ID> TimeoutCert::signers()
{
	std::vector<ID> signers;

	for (auto &signature : m_signatures)
	{
		signers.push_back(signature.signer());
	}

	return signers;
}

Signature Crypto::sign(Hash msg_hash)
{
	auto signer = Botan::PK_Signer(this->m_key, Botan::system_rng(), "Raw");
	auto signature_bytes = signer.sign_message(msg_hash.begin(), msg_hash.size(), Botan::system_rng());
	return Signature(this->m_id, std::move(signature_bytes));
}

Crypto::VerifyResult Crypto::verify(const QuorumCert &qc, int quorum_size)
{
	int num_ok = 0;
	for (auto sig : qc.m_signatures)
	{
		if (!verify(sig, qc.block_hash()))
		{
			continue;
		}

		num_ok++;
		if (num_ok >= quorum_size)
		{
			return VerifyResult(VerifyResult::OK);
		}
	}

	return VerifyResult(VerifyResult::NOT_A_QUORUM,
	                    fmt::format("got only {} of {} required signatures", num_ok, quorum_size));
}

Crypto::VerifyResult Crypto::verify(const Signature &sig, Hash msg_hash)
{
	auto peer = m_peers->find(sig.signer());
	if (!peer)
	{
		return VerifyResult(VerifyResult::PEER_NOT_FOUND, fmt::format("Peer with id '{}' not found.", sig.signer()));
	}

	auto verifier = Botan::PK_Verifier(peer->public_key(), "Raw");
	if (verifier.verify_message(&msg_hash.front(), msg_hash.size(), &sig.m_signature.front(), sig.m_signature.size()))
	{
		return VerifyResult(VerifyResult::OK);
	}

	return VerifyResult(VerifyResult::INVALID_SIGNATURE);
}

Crypto::Crypto(ID id, Botan::ECDSA_PrivateKey key, std::shared_ptr<Peers> peers) : m_id(id), m_key(key), m_peers(peers)
{
}

Crypto::VerifyResult::Kind Crypto::VerifyResult::kind()
{
	return m_kind;
}

bool Crypto::VerifyResult::ok()
{
	return m_kind == Crypto::VerifyResult::OK;
}

std::string Crypto::VerifyResult::message()
{
	return m_message;
}

Crypto::VerifyResult::operator bool()
{
	return ok();
}

Crypto::VerifyResult::VerifyResult(Crypto::VerifyResult::Kind kind, std::string message)
    : m_kind(kind), m_message(message)
{
}

} // namespace HotStuff
