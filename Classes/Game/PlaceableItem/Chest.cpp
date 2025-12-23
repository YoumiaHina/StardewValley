#include "Game/PlaceableItem/Chest.h"

using namespace cocos2d;

namespace Game {

// 返回箱子在世界坐标中的占用矩形（宽度 1 格，高度 2 格）。
// - 这里直接复用 PlaceableItemBase 提供的“标准 1x2”矩形算法。
Rect Chest::placeRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return PlaceableItemBase::standard1x2PlaceRect(pos, s);
}

// 返回用于角色/环境碰撞的矩形，仅覆盖箱子底半部分。
// - 上半部分仅用于视觉表现，不参与“挡路”，避免角色头顶被挡住。
Rect Chest::collisionRect() const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return PlaceableItemBase::standardBottomHalfCollisionRect(pos, s);
}

// 工具函数封装：便于外部调用统一使用 chestRect 接口。
Rect chestRect(const Chest& chest) {
    return chest.placeRect();
}

// 工具函数封装：便于外部调用统一使用 chestCollisionRect 接口。
Rect chestCollisionRect(const Chest& chest) {
    return chest.collisionRect();
}

// 判断玩家在指定位置附近是否接近任意箱子，用于交互/种地避让。
// - 通过 PlaceableItemBase::isNearAny 统一实现“扩展矩形 + containsPoint”逻辑。
bool isNearAnyChest(const Vec2& playerWorldPos, const std::vector<Chest>& chests) {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float margin = s * 0.4f;
    return PlaceableItemBase::isNearAny<Chest>(
        playerWorldPos,
        chests,
        [](const Chest& c) { return chestRect(c); },
        margin);
}

} // namespace Game
