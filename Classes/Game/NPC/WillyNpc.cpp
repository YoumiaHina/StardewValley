#include "Game/NPC/WillyNpc.h"

namespace Game {

const char* Willy::name() const { return "Willy"; }

const char* Willy::texturePath() const { return "NPC/Willy/Willy-standing.png"; }

int Willy::friendshipGainForGift(ItemType itemType) const {
  switch (itemType) {
    case ItemType::Fish: return 60;
    case ItemType::Corn: return 25;
    default: return 20;
  }
}

}  // namespace Game

