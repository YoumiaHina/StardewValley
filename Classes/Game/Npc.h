#pragma once

#include <string>
#include <vector>
#include "cocos2d.h"
#include "Game/Item.h"

namespace Game {

enum class NpcId {
    NPC1 = 1,
    NPC2 = 2
};

struct NpcDefinition {
    NpcId id;
    std::string name;
    std::string texturePath;
};

const std::vector<NpcDefinition>& allNpcDefinitions();
const NpcDefinition* npcDefinitionById(NpcId id);

int npcFriendshipGainForGift(NpcId id, ItemType itemType);

} // namespace Game

