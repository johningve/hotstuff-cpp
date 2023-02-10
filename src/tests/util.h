#include <botan/ecdsa.h>

#include "../blockchain.h"
#include "../crypto.h"
#include "../types.h"

using namespace HotStuff;

Botan::ECDSA_PrivateKey gen_key();

std::pair<std::shared_ptr<Peers>, std::unordered_map<ID, Botan::ECDSA_PrivateKey>> make_peers(int num_peers = 4,
                                                                                              ID first_id = 1);

QuorumCert make_qc(std::shared_ptr<Peers> peers, const std::unordered_map<ID, Botan::ECDSA_PrivateKey> &keys,
                   std::vector<ID> signers = {2, 3, 4}, Hash hash = GENESIS.hash(), Round round = 1);
