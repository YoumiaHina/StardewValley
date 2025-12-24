#include "Game/NPC/PierreNpc.h"

namespace Game {

const char* Pierre::name() const { return "Pierre"; }

const char* Pierre::texturePath() const { return "NPC/Pierre/Pierre - standing.png"; }

// Pierre 赠礼好感规则：
// - 当前实现较为保守，所有礼物统一给予固定好感。
int Pierre::friendshipGainForGift(ItemType) const {
  return 10;
}

}  // namespace Game
