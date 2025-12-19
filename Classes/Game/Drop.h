#pragma once

#include <vector>
#include "cocos2d.h"
#include "Game/Item.h"
#include "Game/GameConfig.h"
#include "Game/Inventory.h"

namespace Game {

class Drop {
public:
    ItemType type;
    cocos2d::Vec2 pos;
    int qty;

    static void renderDrops(const std::vector<Drop>& drops,
                            cocos2d::Node* root,
                            cocos2d::DrawNode* draw);

    static void collectDropsNear(const cocos2d::Vec2& playerWorldPos,
                                 std::vector<Drop>& drops,
                                 Game::Inventory* inv);
};

inline int toolDropRaw(Game::ToolKind tk) {
    return 10000 + static_cast<int>(tk);
}

inline bool isToolDropRaw(int raw) {
    return raw >= 10000;
}

inline Game::ToolKind toolKindFromDropRaw(int raw) {
    return static_cast<Game::ToolKind>(raw - 10000);
}

} // namespace Game
