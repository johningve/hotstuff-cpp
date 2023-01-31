#include <botan/sha2_32.h>

#include "crypto.h"

namespace HotStuff
{

ID Signature::signer()
{
	return this->m_signer;
}

bool Crypto::verify(const QuorumCert &qc)
{
	return true;
}

bool Crypto::verify(const Signature &sig, const std::vector<uint8_t> msg)
{
	auto verifier = new Botan::PK_Verifier(this->m_key, "ECDSA(SHA-256)");
	return verifier->verify_message(msg, sig.m_signature);
}

Crypto::Crypto(Botan::ECDSA_PrivateKey key) : m_key(key)
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

} // namespace HotStuff
