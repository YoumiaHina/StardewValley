#include "Game/NPC/AbigailNpc.h"

namespace Game {

const char* Abigail::name() const { return "Abigail"; }

const char* Abigail::texturePath() const { return "NPC/Abigail/Abigail-standing.png"; }

// Abigail 赠礼好感规则：
// - 草莓给予最高加成；
// - 其次是各类鱼与海产；
// - 其它物品统一给予较低的基础好感。
int Abigail::friendshipGainForGift(ItemType itemType) const {
  switch (itemType) {
    case ItemType::Strawberry: return 55;
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
      return 40;
    case ItemType::Blueberry: return 45;
    default: return 20;
  }
}

}  // namespace Game

