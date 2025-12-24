#pragma once

#include <vector>
#include "cocos2d.h"
#include "Game/Item.h"
#include "Game/GameConfig.h"
#include "Game/Inventory.h"

namespace Game {

// Drop：地面掉落物的一条记录。
// - type：物品类型（ItemType），决定渲染贴图与拾取结果；
// - pos ：世界坐标位置，通常位于地图上的某个点；
// - qty ：堆叠数量。
// renderDrops      ：根据掉落列表在场景中渲染对应精灵/调试形状；
// collectDropsNear ：检测玩家附近掉落并尝试吸入背包。
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
