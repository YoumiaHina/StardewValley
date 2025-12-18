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

} // namespace Game
