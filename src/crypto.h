#pragma once

#include <botan/ecdsa.h>
#include <botan/pubkey.h>
#include <cereal/access.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>

#include "types.h"

namespace HotStuff
{

typedef std::array<uint8_t, 32> Hash;

class Crypto;

class Signature
{
  public:
	ID signer();

  private:
	friend class Crypto;
	friend class cereal::access;

	std::vector<uint8_t> m_signature;
	ID m_signer;

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

static const QuorumCert genesis_qc = QuorumCert(Hash(), Round(), std::vector<Signature>());

class Crypto
{
	Botan::ECDSA_PrivateKey m_key;

  public:
	Crypto(Botan::ECDSA_PrivateKey key);

	bool verify(const QuorumCert &qc);
	bool verify(const Signature &sig, const std::vector<uint8_t> msg);
};
} // namespace HotStuff

#include "util/array_hasher.h" // specialization needed to allow Hash to be usable in unordered_map

namespace std
{
template <> struct hash<HotStuff::Hash>
{
	size_t operator()(const HotStuff::Hash &a) const noexcept
	{
		return hash<array<uint8_t, 32>>{}(reinterpret_cast<const array<uint8_t, 32> &>(a));
	}
};
} // namespace std
