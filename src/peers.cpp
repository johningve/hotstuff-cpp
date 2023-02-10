#include "peers.h"

namespace HotStuff
{
Peer::Peer(ID id, Botan::ECDSA_PublicKey key) : m_id(id), m_key(key)
{
}

ID Peer::id()
{
	return m_id;
}

Botan::ECDSA_PublicKey Peer::public_key()
{
	return m_key;
}

std::optional<Peer> Peers::find(ID id)
{
	auto peer_entry = m_peers.find(id);
	if (peer_entry == m_peers.end())
	{
		return std::nullopt;
	}
	return peer_entry->second;
}

void Peers::add(Peer peer)
{
	m_peers.insert({peer.id(), peer});
}

} // namespace HotStuff
