#include <catch2/catch_test_macros.hpp>

#include "util.h"

Botan::ECDSA_PrivateKey gen_key()
{
	Botan::ECDSA_PrivateKey key(Botan::system_rng(), Botan::EC_Group("secp256k1"));
	return key;
}

std::pair<std::shared_ptr<Peers>, std::unordered_map<ID, Botan::ECDSA_PrivateKey>> make_peers(int num_peers,
                                                                                              ID first_id)
{
	std::unordered_map<ID, Botan::ECDSA_PrivateKey> keys;
	auto peers = std::make_shared<Peers>();

	for (ID i = first_id; i < first_id + num_peers; i++)
	{
		auto key = gen_key();
		keys.insert({i, key});
		peers->add({i, key});
	}

	return {peers, keys};
}

QuorumCert make_qc(std::shared_ptr<Peers> peers, const std::unordered_map<ID, Botan::ECDSA_PrivateKey> &keys,
                   std::vector<ID> signers, Hash hash, Round round)
{
	std::vector<Signature> sigs;
	for (ID i : signers)
	{
		auto key = keys.find(i);
		REQUIRE(key != keys.end());

		Crypto crypto(i, key->second, peers);

		sigs.push_back(crypto.sign(hash));
	}

	return QuorumCert(hash, round, std::move(sigs));
}
