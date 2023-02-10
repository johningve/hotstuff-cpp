#pragma once

#include <botan/ecdsa.h>
#include <botan/pubkey.h>
#include <cereal/access.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>

#include "util/array_hasher.h" // specialization needed to allow Hash to be usable in unordered_map

#include "peers.h"
#include "types.h"

namespace HotStuff
{

typedef std::array<uint8_t, 32> Hash;

class Crypto;

class Signature
{
  public:
	ID signer() const;

	// This public constructor is needed because we want to serialize QuorumCert,
	// which has a vector of signatures. Cereal uses vector::resize, which needs
	// to be able to construct new objects. This should be removed.
	// see: https://github.com/USCiLab/cereal/issues/704
	[[deprecated]] Signature();

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
	Hash block_hash() const;
	Round round() const;
	std::vector<ID> signers() const;

	QuorumCert(Hash block, Round round, std::vector<Signature> signatures);
	QuorumCert(const QuorumCert &other);

  private:
	friend class Crypto;
	friend class cereal::access;

	std::vector<Signature> m_signatures;
	Hash m_block;
	Round m_round;

	template <class Archive> void serialize(Archive &archive)
	{
		archive(m_signatures, m_block, m_round);
	}
};

class TimeoutCert
{
  public:
	Round round();

	TimeoutCert(Round round, std::vector<Signature> signatures);
	TimeoutCert(const TimeoutCert &other);

  private:
	friend class Crypto;
	friend class cereal::access;

	std::vector<Signature> m_signatures;
	Round m_round;

	std::vector<ID> signers();

	template <class Archive> void serialize(Archive &archive)
	{
		archive(m_signatures, m_round);
	}
};

const QuorumCert GENESIS_QC = QuorumCert(Hash(), Round(), std::vector<Signature>());

class Crypto
{
  public:
	class VerifyResult;

	Crypto(ID id, Botan::ECDSA_PrivateKey key, std::shared_ptr<Peers> m_peers);

	Signature sign(Hash msg_hash);
	VerifyResult verify(const QuorumCert &qc, int quorum_size);
	VerifyResult verify(const Signature &sig, Hash msg_hash);

	class VerifyResult
	{
	  public:
		enum Kind
		{
			OK,
			PEER_NOT_FOUND,
			INVALID_SIGNATURE,
			NOT_A_QUORUM,
		};

		Kind kind();
		bool ok();
		std::string message();

		operator bool();

	  private:
		friend class Crypto;

		VerifyResult(Kind kind, std::string message = "");
		Kind m_kind;
		std::string m_message;
	};

  private:
	ID m_id;
	Botan::ECDSA_PrivateKey m_key;
	std::shared_ptr<Peers> m_peers;
};

} // namespace HotStuff
