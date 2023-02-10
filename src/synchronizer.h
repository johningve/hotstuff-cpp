#pragma once

#include <variant>

#include "crypto.h"
#include "types.h"

namespace HotStuff
{

class Synchronizer
{
  public:
	Round round();
	void update(std::variant<QuorumCert, TimeoutCert> cert);
};

} // namespace HotStuff
