#include "Controllers/Map/BeachMapController.h"
#include "Controllers/TileSelector.h"
#include "Game/GameConfig.h"

using namespace cocos2d;

namespace Controllers {

void BeachMapController::addActorToMap(Node* node, int zOrder) {
    if (_map && _map->getTMX()) {
        _map->getTMX()->addChild(node, 20);
    } else if (_worldNode) {
        _worldNode->addChild(node, zOrder);
    }
}

Vec2 BeachMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    float s = tileSize();
    Size ms = _map->getContentSize();
    float minX = s * 0.5f;
    float minY = s * 0.5f;
    float maxX = ms.width - s * 0.5f;
    float maxY = ms.height - s * 0.5f;
    Vec2 candidate(std::max(minX, std::min(next.x, maxX)),
                   std::max(minY, std::min(next.y, maxY)));
    Vec2 footX(candidate.x, current.y);
    if (_map && _map->collides(footX, radius)) {
        candidate.x = current.x;
    }
    Vec2 footY(current.x, candidate.y);
    if (_map && _map->collides(footY, radius)) {
        candidate.y = current.y;
    }
    return candidate;
}

std::pair<int,int> BeachMapController::targetTile(const Vec2& playerPos, const Vec2& lastDir) const {
    return TileSelector::selectForwardTile(
        playerPos,
        lastDir,
        [this](const Vec2& p, int& c, int& r){ worldToTileIndex(p, c, r); },
        [this](int c, int r){ return inBounds(c, r); },
        _hasLastClick,
        _lastClickWorldPos,
        [this](int c, int r){ return tileToWorld(c, r); });
}

void BeachMapController::updateCursor(const Vec2& playerPos, const Vec2& lastDir) {
    if (!_cursor) {
        _cursor = DrawNode::create();
        if (_map && _map->getTMX()) {
            _map->getTMX()->addChild(_cursor, 21);
        } else if (_worldNode) {
            _worldNode->addChild(_cursor, 21);
        }
    }
    if (!_cursor) return;
    TileSelector::drawFanCursor(
        _cursor,
        playerPos,
        lastDir,
        [this](const Vec2& p, int& c, int& r) { worldToTileIndex(p, c, r); },
        [this](int c, int r) { return inBounds(c, r); },
        [this](int c, int r) { return tileToWorld(c, r); },
        tileSize());
}

} // namespace Controllers

