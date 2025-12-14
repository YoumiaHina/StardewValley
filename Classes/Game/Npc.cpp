#include "Game/Npc.h"

namespace Game {

const std::vector<NpcId>& NpcRegistry::allIds() {
  static std::vector<NpcId> ids = {NpcId::kAbigail, NpcId::kWilly};
  return ids;
}

std::unique_ptr<NpcBase> NpcRegistry::create(NpcId id) {
  switch (id) {
    case NpcId::kAbigail:
      return std::make_unique<Abigail>();
    case NpcId::kWilly:
      return std::make_unique<Willy>();
  }
  return nullptr;
}

}  // namespace Game
