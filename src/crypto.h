#pragma once

#include <botan/ecdsa.h>
#include <botan/pubkey.h>
#include <cereal/access.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>

#include "util/array_hasher.h" // specialization needed to allow Hash to be usable in unordered_map

#include "types.h"

namespace HotStuff
{

typedef std::array<uint8_t, 32> Hash;

class Crypto;

class Signature
{
  public:
	ID signer();

	Signature();

  private:
	friend class Crypto;
	friend class cereal::access;

	std::vector<uint8_t> m_signature;
	ID m_signer;

	Signature(ID signer, std::vector<uint8_t> signature);

	template <class Archive> void serialize(Archive &archive)
	{
		archive(m_signature, m_signer);
	}
};

class QuorumCert
{
  public:
	Hash block();
	Round round();

	QuorumCert(Hash block, Round round, std::vector<Signature> signatures);
	QuorumCert(const QuorumCert &other);

  private:
	friend class Crypto;
	friend class cereal::access;

	std::vector<Signature> m_signatures;
	Hash m_block;
	Round m_round;

	std::vector<ID> signers();

	template <class Archive> void serialize(Archive &archive)
	{
		archive(m_signatures, m_block, m_round);
	}
};

const QuorumCert genesis_qc = QuorumCert(Hash(), Round(), std::vector<Signature>());

class Crypto
{
	ID m_id;
	Botan::ECDSA_PrivateKey m_key;

  public:
	Crypto(ID id, Botan::ECDSA_PrivateKey key);

	Signature sign(Hash msg_hash);
	bool verify(const QuorumCert &qc);
	bool verify(const Signature &sig, Hash msg_hash);
};
} // namespace HotStuff
