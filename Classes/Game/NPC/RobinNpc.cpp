#include "Game/NPC/RobinNpc.h"

namespace Game {

const char* Robin::name() const { return "Robin"; }

const char* Robin::texturePath() const { return "NPC/Robin - stanging.png"; }

int Robin::friendshipGainForGift(ItemType) const {
  return 10;
}

}  // namespace Game
