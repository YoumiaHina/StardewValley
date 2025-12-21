#include "Game/NPC/WillyNpc.h"

namespace Game {

const char* Willy::name() const { return "Willy"; }

const char* Willy::texturePath() const { return "NPC/Willy/Willy-standing.png"; }

int Willy::friendshipGainForGift(ItemType itemType) const {
  switch (itemType) {
    case ItemType::Fish:
    case ItemType::Carp:
    case ItemType::BreamFish:
    case ItemType::Sardine:
    case ItemType::Salmon:
    case ItemType::RainbowTrout:
    case ItemType::MidnightCarp:
    case ItemType::LargemouthBass:
    case ItemType::Sturgeon:
    case ItemType::SmallmouthBass:
    case ItemType::Tilapia:
    case ItemType::Tuna:
    case ItemType::Globefish:
    case ItemType::Anchovy:
    case ItemType::BlueDiscus:
    case ItemType::Clam:
    case ItemType::Crab:
    case ItemType::Lobster:
    case ItemType::Shrimp:
      return 60;
    case ItemType::Corn: return 25;
    default: return 20;
  }
}

}  // namespace Game

