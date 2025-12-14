#include "Game/NPC/NpcBase.h"

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

const char* Willy::name() const { return "Willy"; }
const char* Willy::texturePath() const { return "NPC/Willy-standing.png"; }
int Willy::friendshipGainForGift(ItemType itemType) const {
  switch (itemType) {
    case ItemType::Fish: return 60;
    case ItemType::Corn: return 25;
    default: return 10;
  }
}

}  // namespace Game
