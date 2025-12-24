#include "Game/NPC/WillyNpc.h"

namespace Game {

const char* Willy::name() const { return "Willy"; }

const char* Willy::texturePath() const { return "NPC/Willy/Willy-standing.png"; }

// Willy 赠礼好感规则：
// - 对各种鱼与海产有较高好感；
// - 其次是玉米等少量农作物；
// - 其它物品给予中等基础好感。
int Willy::friendshipGainForGift(ItemType itemType) const {
  switch (itemType) {
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
    case ItemType::Corn: return 45;
    default: return 30;
  }
}

}  // namespace Game

