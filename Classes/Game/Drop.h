#pragma once

#include <vector>
#include "cocos2d.h"
#include "Game/Item.h"
#include "Game/GameConfig.h"
#include "Game/Inventory.h"

namespace Game {

// 掉落物实体（简单结构）
struct Drop {
    ItemType type;
    cocos2d::Vec2 pos;
    int qty;
};

} // namespace Game

namespace Controllers {

namespace DropHelper {

inline void renderDrops(const std::vector<Game::Drop>& drops,
                        cocos2d::Node* root,
                        cocos2d::DrawNode* draw) {
    if (!draw) return;
    draw->clear();
    if (root) {
        root->removeAllChildren();
    }
    for (const auto& d : drops) {
        bool usedSprite = false;
        if (root) {
            std::string path = Game::itemIconPath(d.type);
            if (!path.empty()) {
                auto spr = cocos2d::Sprite::create(path);
                if (spr && spr->getTexture()) {
                    float radius = GameConfig::DROP_DRAW_RADIUS;
                    auto cs = spr->getContentSize();
                    if (cs.width > 0 && cs.height > 0) {
                        float targetSize = radius * 2.0f;
                        float sx = targetSize / cs.width;
                        float sy = targetSize / cs.height;
                        float scale = std::min(sx, sy);
                        spr->setScale(scale);
                    }
                    spr->setPosition(d.pos);
                    root->addChild(spr);
                    usedSprite = true;
                }
            }
        }
        if (!usedSprite) {
            draw->drawSolidCircle(d.pos, GameConfig::DROP_DRAW_RADIUS, 0.0f, 12, Game::itemColor(d.type));
            draw->drawCircle(d.pos, GameConfig::DROP_DRAW_RADIUS, 0.0f, 12, false, cocos2d::Color4F(0,0,0,0.4f));
        }
    }
}

inline void collectDropsNear(const cocos2d::Vec2& playerWorldPos,
                             std::vector<Game::Drop>& drops,
                             Game::Inventory* inv) {
    if (!inv) return;
    float radius = GameConfig::DROP_PICK_RADIUS;
    float r2 = radius * radius;
    std::vector<Game::Drop> kept;
    kept.reserve(drops.size());
    for (auto& d : drops) {
        float dist2 = playerWorldPos.distanceSquared(d.pos);
        if (dist2 <= r2) {
            int leftover = inv->addItems(d.type, d.qty);
            if (leftover > 0) {
                Game::Drop nd = d;
                nd.qty = leftover;
                kept.push_back(nd);
            }
        } else {
            kept.push_back(d);
        }
    }
    drops.swap(kept);
}

}

} // namespace Controllers
