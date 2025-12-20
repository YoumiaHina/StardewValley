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

inline int toolDropRaw(Game::ToolKind tk, int level) {
    if (level < 0) level = 0;
    if (level > 3) level = 3;
    int base = static_cast<int>(tk);
    return 10000 + level * 100 + base;
}

inline int toolDropRaw(Game::ToolKind tk) {
    return toolDropRaw(tk, 0);
}

inline bool isToolDropRaw(int raw) {
    return raw >= 10000;
}

inline Game::ToolKind toolKindFromDropRaw(int raw) {
    int base = raw - 10000;
    int kindInt = base % 100;
    return static_cast<Game::ToolKind>(kindInt);
}

inline int toolLevelFromDropRaw(int raw) {
    int base = raw - 10000;
    int lv = base / 100;
    if (lv < 0) lv = 0;
    if (lv > 3) lv = 3;
    return lv;
}


} // namespace Game
