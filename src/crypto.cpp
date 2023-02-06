#include <botan/sha2_32.h>

#include "crypto.h"

namespace HotStuff
{

ID Signature::signer()
{
	return this->m_signer;
}

Signature::Signature()
{
}

Signature::Signature(ID signer, std::vector<uint8_t> signature) : m_signer(signer), m_signature(signature)
{
}

Hash QuorumCert::block()
{
	return m_block;
}

Round QuorumCert::round()
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

std::vector<ID> QuorumCert::signers()
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

bool Crypto::verify(const QuorumCert &qc)
{
	return true;
}

bool Crypto::verify(const Signature &sig, Hash msg_hash)
{
	auto verifier = Botan::PK_Verifier(this->m_key, "Raw");
	return verifier.verify_message(&msg_hash.front(), msg_hash.size(), &sig.m_signature.front(),
	                               sig.m_signature.size());
}

Crypto::Crypto(ID id, Botan::ECDSA_PrivateKey key) : m_id(id), m_key(key)
{
}

} // namespace HotStuff
