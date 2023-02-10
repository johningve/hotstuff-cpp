#pragma once

#include <botan/ecdsa.h>
#include <optional>
#include <unordered_map>

#include "types.h"

namespace HotStuff
{

class Peer
{
  public:
	Peer(ID id, Botan::ECDSA_PublicKey key);

	ID id();
	Botan::ECDSA_PublicKey public_key();

  private:
	ID m_id;
	Botan::ECDSA_PublicKey m_key;
};

class Peers
{
  public:
	std::optional<Peer> find(ID id);
	void add(Peer peer);

  private:
	std::unordered_map<ID, Peer> m_peers;
};

} // namespace HotStuff
