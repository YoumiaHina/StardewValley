#include "Controllers/RoomMapController.h"
#include "cocos2d.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace Controllers {

void RoomMapController::init() {
    if (!_worldNode) return;
    _roomDraw = DrawNode::create();
    _worldNode->addChild(_roomDraw, 0);

    _roomDraw->clear();
    float roomW = 600.0f;
    float roomH = 400.0f;
    _roomRect = Rect(0, 0, roomW, roomH);

    float doorW = 100.0f;
    float doorH = 24.0f;
    float dx = _roomRect.getMidX() - doorW * 0.5f;
    float dy = _roomRect.getMinY() + 2.0f;
    _doorRect = Rect(dx, dy, doorW, doorH);

    _roomDraw->drawSolidRect(_roomRect.origin,
                             Vec2(_roomRect.getMaxX(), _roomRect.getMaxY()),
                             Color4F(0.85f, 0.75f, 0.60f, 1.0f));

    Vec2 tl(_roomRect.getMinX(), _roomRect.getMaxY());
    Vec2 tr(_roomRect.getMaxX(), _roomRect.getMaxY());
    Vec2 bl(_roomRect.getMinX(), _roomRect.getMinY());
    Vec2 br(_roomRect.getMaxX(), _roomRect.getMinY());
    Color4F wall(0.f, 0.f, 0.f, 0.55f);
    _roomDraw->drawLine(tl, tr, wall);
    _roomDraw->drawLine(tl, bl, wall);
    _roomDraw->drawLine(tr, br, wall);
    Vec2 dl(_doorRect.getMinX(), _roomRect.getMinY());
    Vec2 dr(_doorRect.getMaxX(), _roomRect.getMinY());
    _roomDraw->drawLine(bl, dl, wall);
    _roomDraw->drawLine(dr, br, wall);

    _roomDraw->drawSolidRect(_doorRect.origin,
                             Vec2(_doorRect.getMaxX(), _doorRect.getMaxY()),
                             Color4F(0.75f, 0.75f, 0.20f, 0.75f));

    float bedW = 120.0f, bedH = 60.0f;
    _bedRect = Rect(_roomRect.getMinX() + 24.0f,
                    _roomRect.getMaxY() - bedH - 24.0f,
                    bedW, bedH);
    _roomDraw->drawSolidRect(_bedRect.origin,
                             Vec2(_bedRect.getMaxX(), _bedRect.getMaxY()),
                             Color4F(0.85f, 0.85f, 0.95f, 1.0f));
    _roomDraw->drawRect(_bedRect.origin,
                        Vec2(_bedRect.getMaxX(), _bedRect.getMaxY()),
                        Color4F(0.2f, 0.2f, 0.3f, 1.0f));

    _chestDraw = DrawNode::create();
    _worldNode->addChild(_chestDraw, 1);
}

Size RoomMapController::getContentSize() const {
    return Size(_roomRect.size.width, _roomRect.size.height);
}

Vec2 RoomMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    float pad = 16.0f;
    Vec2 clamped = next;
    clamped.x = std::max(_roomRect.getMinX() + pad, std::min(_roomRect.getMaxX() - pad, clamped.x));
    clamped.y = std::max(_roomRect.getMinY() + pad, std::min(_roomRect.getMaxY() - pad, clamped.y));
    return clamped;
}

bool RoomMapController::isNearDoor(const Vec2& playerWorldPos) const {
    return _doorRect.containsPoint(playerWorldPos);
}

bool RoomMapController::isNearChest(const Vec2& playerWorldPos) const {
    float maxDist = 40.0f; // 室内更紧凑
    for (const auto& ch : _chests) {
        if (playerWorldPos.distance(ch.pos) <= maxDist) return true;
    }
    return false;
}

void RoomMapController::addActorToMap(cocos2d::Node* node, int zOrder) {
    if (_worldNode) {
        _worldNode->addChild(node, zOrder);
    }
}

} // namespace Controllers