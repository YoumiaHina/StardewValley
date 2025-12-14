#include "Game/NPC/AbigailNpc.h"

namespace Game {

const char* Abigail::name() const { return "Abigail"; }

const char* Abigail::texturePath() const { return "NPC/Abigail-standing.png"; }

int Abigail::friendshipGainForGift(ItemType itemType) const {
  switch (itemType) {
    case ItemType::Strawberry: return 55;
    case ItemType::Fish: return 40;
    case ItemType::Blueberry: return 35;
    default: return 10;
  }
}

}  // namespace Game

