#pragma once

#include "cocos2d.h"
#include <vector>
#include <functional>

namespace Game {

class PlaceableItemBase {
public:
    cocos2d::Vec2 pos;

    PlaceableItemBase();
    virtual ~PlaceableItemBase();

    virtual cocos2d::Rect placeRect() const = 0;
    virtual cocos2d::Rect collisionRect() const = 0;
    virtual int maxPerArea() const = 0;

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

    static cocos2d::Rect standard1x2PlaceRect(const cocos2d::Vec2& center,
                                              float tileSize);

    static cocos2d::Rect standardBottomHalfCollisionRect(const cocos2d::Vec2& center,
                                                         float tileSize);

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
