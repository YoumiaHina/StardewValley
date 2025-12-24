#include "Game/NPC/RobinNpc.h"

namespace Game {

const char* Robin::name() const { return "Robin"; }

const char* Robin::texturePath() const { return "NPC/Robin/Robin - stanging.png"; }

// Robin 赠礼好感规则：
// - 目前所有礼物统一给予基础好感值，后续可按物品类型细化。
int Robin::friendshipGainForGift(ItemType) const {
  return 10;
}

}  // namespace Game
