#include "Controllers/Systems/ChestController.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace Controllers {

void ChestController::attachTo(Node* parentNode, int zOrder) {
    _parentNode = parentNode;
    if (!_parentNode) return;
    _chestDraw = DrawNode::create();
    _parentNode->addChild(_chestDraw, zOrder);
}

void ChestController::syncLoad() {
    auto& ws = Game::globalState();
    if (_isFarm) {
        _chests = ws.farmChests;
    } else {
        _chests = ws.houseChests;
    }
}

const std::vector<Game::Chest>& ChestController::chests() const {
    return _chests;
}

std::vector<Game::Chest>& ChestController::chests() {
    return _chests;
}

bool ChestController::isNearChest(const Vec2& worldPos) const {
    return Game::isNearAnyChest(worldPos, _chests);
}

bool ChestController::collides(const Vec2& worldPos) const {
    for (const auto& ch : _chests) {
        if (Game::chestCollisionRect(ch).containsPoint(worldPos)) {
            return true;
        }
    }
    return false;
}

void ChestController::refreshVisuals() {
    if (!_chestDraw) return;
    _chestDraw->clear();
    _chestDraw->removeAllChildren();
    for (const auto& ch : _chests) {
        auto r = Game::chestRect(ch);
        Vec2 center(r.getMidX(), r.getMidY());
        auto spr = Sprite::create("Chest.png");
        if (spr && spr->getTexture()) {
            auto cs = spr->getContentSize();
            if (cs.width > 0 && cs.height > 0) {
                float sx = r.size.width / cs.width;
                float sy = r.size.height / cs.height;
                float scale = std::min(sx, sy);
                spr->setScale(scale);
            }
            spr->setPosition(center);
            _chestDraw->addChild(spr);
        } else {
            Vec2 a(r.getMinX(), r.getMinY());
            Vec2 b(r.getMaxX(), r.getMinY());
            Vec2 c(r.getMaxX(), r.getMaxY());
            Vec2 d(r.getMinX(), r.getMaxY());
            Vec2 v[4] = { a, b, c, d };
            _chestDraw->drawSolidPoly(v, 4, Color4F(0.6f,0.4f,0.2f,0.9f));
        }
    }
}

}

