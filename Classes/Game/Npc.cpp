#include "Game/Npc.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace Game {

static std::vector<NpcDefinition> makeDefinitions() {
    std::vector<NpcDefinition> defs;
    NpcDefinition a;
    a.id = NpcId::NPC1;
    a.name = "NPC1";
    a.texturePath = "Farmer/NPC1-standing.png";
    defs.push_back(a);
    NpcDefinition b;
    b.id = NpcId::NPC2;
    b.name = "NPC2";
    b.texturePath = "Farmer/NPC2-standing.png";
    defs.push_back(b);
    return defs;
}

const std::vector<NpcDefinition>& allNpcDefinitions() {
    static std::vector<NpcDefinition> defs = makeDefinitions();
    return defs;
}

const NpcDefinition* npcDefinitionById(NpcId id) {
    const auto& defs = allNpcDefinitions();
    for (const auto& d : defs) {
        if (d.id == id) return &d;
    }
    return nullptr;
}

static int baseGainForItem(ItemType itemType) {
    switch (itemType) {
        case ItemType::Fish: return 40;
        case ItemType::Parsnip: return 30;
        case ItemType::Blueberry: return 25;
        case ItemType::Eggplant: return 25;
        case ItemType::Corn: return 20;
        case ItemType::Strawberry: return 35;
        default: return 5;
    }
}

int npcFriendshipGainForGift(NpcId id, ItemType itemType) {
    int gain = baseGainForItem(itemType);
    switch (id) {
        case NpcId::NPC1:
            if (itemType == ItemType::Fish) gain += 20;
            break;
        case NpcId::NPC2:
            if (itemType == ItemType::Strawberry) gain += 20;
            break;
    }
    if (gain < 0) gain = 0;
    return gain;
}

} // namespace Game
