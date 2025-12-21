#pragma once

#include "cocos2d.h"
#include <vector>
#include <functional>

namespace Game {

// PlaceableItemBase：所有“可放置物体”（箱子、熔炉等）的抽象基类。
// - 职责：
//   1. 提供统一的世界坐标位置 pos。
//   2. 定义占用矩形/碰撞矩形/maxPerArea 等通用接口。
//   3. 提供常用的矩形工具与放置/碰撞判定模板算法。
// - 协作：
//   - Chest/Furnace 等具体实体从该类继承实现 placeRect/collisionRect/maxPerArea。
//   - ChestController/FurnaceController 等系统通过静态工具函数实现放置/碰撞逻辑。
// - 约束：此类只做几何与基础算法，不承载具体业务（掉落、背包等高层逻辑）。
class PlaceableItemBase {
public:
cocos2d::Vec2 pos;          // 世界坐标下物体的中心位置

PlaceableItemBase();
virtual ~PlaceableItemBase();

// 返回用于放置与渲染的占用矩形（通常覆盖整个物体）。
virtual cocos2d::Rect placeRect() const = 0;
// 返回用于角色/环境碰撞检测的矩形（可只覆盖物体下半部分）。
virtual cocos2d::Rect collisionRect() const = 0;
// 返回单个地图区域允许放置的该类物体最大数量，用于系统层做数量限制。
virtual int maxPerArea() const = 0;

// 判定 playerWorldPos 附近是否“接近任意一个物体”，常用于交互/种地避让。
template<typename T>
static bool isNearAny(const cocos2d::Vec2& playerWorldPos,
                      const std::vector<T>& items,
                      const std::function<cocos2d::Rect(const T&)>& rectFn,
                      float margin) {
    for (const auto& item : items) {
        cocos2d::Rect r = rectFn(item);
        cocos2d::Rect expanded(r.getMinX() - margin,
                               r.getMinY() - margin,
                               r.size.width + margin * 2.0f,
                               r.size.height + margin * 2.0f);
        if (expanded.containsPoint(playerWorldPos)) return true;
    }
    return false;
}

// 判定 worldPos 是否与任意一个物体发生碰撞，常用于地图控制器的障碍检测。
template<typename T>
static bool collidesAny(const cocos2d::Vec2& worldPos,
                        const std::vector<T>& items,
                        const std::function<cocos2d::Rect(const T&)>& rectFn) {
    for (const auto& item : items) {
        cocos2d::Rect r = rectFn(item);
        if (r.containsPoint(worldPos)) return true;
    }
    return false;
}

// 标准 1x2 格占用矩形：以 center 为中心，宽 1 格，高 2 格。
static cocos2d::Rect standard1x2PlaceRect(const cocos2d::Vec2& center,
                                          float tileSize);

// 标准“下半部分”碰撞矩形：用于高于一格的物体，只让下半部参与碰撞。
static cocos2d::Rect standardBottomHalfCollisionRect(const cocos2d::Vec2& center,
                                                     float tileSize);

// 判定在 center 处是否可以放置一个新物体：
// - rectFn：给定实体计算占用矩形。
// - blocked：外部提供的额外阻挡区域（门/床等）；nullptr 表示不做额外阻挡检查。
// - margin：与已有物体的最小间距。
// 注意：这里通过创建一个临时 T 并设置 tmp.pos = center 来复用 rectFn，
//       要求 T 拥有 public 成员 pos（Chest/Furnace 等均满足），在其它类型上使用前需谨慎。
template<typename T>
static bool canPlaceAt(const cocos2d::Vec2& center,
                       const std::vector<T>& items,
                       const std::function<cocos2d::Rect(const T&)>& rectFn,
                       const std::function<bool(const cocos2d::Rect&)>& blocked,
                       float margin) {
    T tmp;
    tmp.pos = center;
    cocos2d::Rect candidateRect = rectFn(tmp);
    if (blocked && blocked(candidateRect)) return false;
    if (isNearAny(center, items, rectFn, margin)) return false;
    return true;
}
};

inline PlaceableItemBase::PlaceableItemBase()
    : pos(cocos2d::Vec2::ZERO) {}

inline PlaceableItemBase::~PlaceableItemBase() = default;

inline cocos2d::Rect PlaceableItemBase::standard1x2PlaceRect(const cocos2d::Vec2& center,
                                                              float tileSize) {
    float s = tileSize;
    float w = s;
    float h = s * 2.0f;
    float minX = center.x - w * 0.5f;
    float minY = center.y - h * 0.5f;
    return cocos2d::Rect(minX, minY, w, h);
}

inline cocos2d::Rect PlaceableItemBase::standardBottomHalfCollisionRect(const cocos2d::Vec2& center,
                                                                         float tileSize) {
    float s = tileSize;
    float w = s;
    float h = s * 2.0f;
    float minX = center.x - w * 0.5f;
    float midY = center.y;
    return cocos2d::Rect(minX, midY, w, h * 0.5f);
}

} // namespace Game
