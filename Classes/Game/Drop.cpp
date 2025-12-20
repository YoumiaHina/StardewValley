#include "Game/Drop.h"
#include "Game/Tool/ToolFactory.h"
#include <algorithm>
#include <string>

using namespace cocos2d;

namespace Game {

static std::string toolDropIconPath(Game::ToolKind tk, int level) {
    auto tool = makeTool(tk);
    if (!tool) return std::string();
    tool->setLevel(level);
    return tool->iconPath();
}

void Drop::renderDrops(const std::vector<Drop>& drops, cocos2d::Node* root, cocos2d::DrawNode* draw) {
    if (!draw) return;
    draw->clear();
    if (root) {
        root->removeAllChildren();
    }
    for (const auto& d : drops) {
        bool usedSprite = false;
        int raw = static_cast<int>(d.type);
        std::string path;
        if (isToolDropRaw(raw)) {
            Game::ToolKind tk = toolKindFromDropRaw(raw);
            int level = toolLevelFromDropRaw(raw);
            path = toolDropIconPath(tk, level);
        } else {
            path = Game::itemIconPath(d.type);
        }
        if (root) {
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
            cocos2d::Color4F color = Game::itemColor(d.type);
            if (isToolDropRaw(raw)) {
                color = cocos2d::Color4F(0.95f, 0.95f, 0.95f, 1.0f);
            }
            draw->drawSolidCircle(d.pos, GameConfig::DROP_DRAW_RADIUS, 0.0f, 12, color);
            draw->drawCircle(d.pos, GameConfig::DROP_DRAW_RADIUS, 0.0f, 12, false, cocos2d::Color4F(0, 0, 0, 0.4f));
        }
    }
}

void Drop::collectDropsNear(const cocos2d::Vec2& playerWorldPos, std::vector<Drop>& drops, Game::Inventory* inv) {
    if (!inv) return;
    float radius = GameConfig::DROP_PICK_RADIUS;
    float r2 = radius * radius;
    std::vector<Drop> kept;
    kept.reserve(drops.size());
    for (auto& d : drops) {
        float dist2 = playerWorldPos.distanceSquared(d.pos);
        if (dist2 <= r2) {
            int raw = static_cast<int>(d.type);
            if (isToolDropRaw(raw)) {
                Game::ToolKind tk = toolKindFromDropRaw(raw);
                int level = toolLevelFromDropRaw(raw);
                bool placed = false;
                std::size_t sz = inv->size();
                for (std::size_t i = 0; i < sz; ++i) {
                    if (inv->isEmpty(i)) {
                        auto tool = Game::makeTool(tk);
                        if (tool) {
                            tool->setLevel(level);
                        }
                        inv->setTool(i, tool);
                        placed = true;
                        break;
                    }
                }
                if (!placed) {
                    kept.push_back(d);
                }
            } else {
                int leftover = inv->addItems(d.type, d.qty);
                if (leftover > 0) {
                    Drop nd = d;
                    nd.qty = leftover;
                    kept.push_back(nd);
                }
            }
        } else {
            kept.push_back(d);
        }
    }
    drops.swap(kept);
}

} // namespace Game
