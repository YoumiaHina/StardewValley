#pragma once

#include "cocos2d.h"
#include "Game/Item.h"

namespace Game {

// 掉落物实体（简单结构）
struct Drop {
    ItemType type;
    cocos2d::Vec2 pos;
    int qty;
};

} // namespace Game