#pragma once

#include <memory>
#include <vector>
#include "Game/NPC/NpcBase.h"

namespace Game {

enum class NpcId { kAbigail = 1, kWilly = 2 };

class NpcRegistry {
 public:
  static const std::vector<NpcId>& allIds();
  static std::unique_ptr<NpcBase> create(NpcId id);
};

}  // namespace Game


