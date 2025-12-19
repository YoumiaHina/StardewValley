#include "Game/NPC/PierreNpc.h"

namespace Game {

const char* Pierre::name() const { return "Pierre"; }

const char* Pierre::texturePath() const { return "NPC/Pierre/Pierre - standing.png"; }

int Pierre::friendshipGainForGift(ItemType) const {
  return 10;
}

}  // namespace Game
