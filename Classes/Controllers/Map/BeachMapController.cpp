#include "Controllers/Map/BeachMapController.h"
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

} // namespace Controllers

