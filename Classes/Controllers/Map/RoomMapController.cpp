#include "Controllers/Map/RoomMapController.h"
#include "cocos2d.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
#include "Game/Map/RoomMap.h"

using namespace cocos2d;

namespace Controllers {

void RoomMapController::init() {
    if (!_worldNode) return;
    _roomMap = Game::RoomMap::create("Maps/farm_room/farm_room.tmx");
    if (_roomMap) {
        _roomMap->setAnchorPoint(Vec2(0,0));
        _roomMap->setPosition(Vec2(0,0));
        _worldNode->addChild(_roomMap, 0);

        if (auto tmx = _roomMap->getTMX()) {
            auto bedBody = tmx->getLayer("BedBody");
            if (bedBody) {
                bedBody->setLocalZOrder(100);
            }
        }

        auto cs = _roomMap->getContentSize();
        _roomRect = Rect(0, 0, cs.width, cs.height);

        cocos2d::Rect door;
        if (_roomMap->getFirstDoorRect(door)) {
            _doorRect = door;
        } else {
            float doorW = 100.0f;
            float doorH = 24.0f;
            float dx = _roomRect.getMidX() - doorW * 0.5f;
            float dy = _roomRect.getMinY() + 2.0f;
            _doorRect = Rect(dx, dy, doorW, doorH);
        }

        const auto& beds = _roomMap->bedRects();
        if (!beds.empty()) {
            float minX = beds[0].getMinX(), minY = beds[0].getMinY();
            float maxX = beds[0].getMaxX(), maxY = beds[0].getMaxY();
            for (size_t i = 1; i < beds.size(); ++i) {
                minX = std::min(minX, beds[i].getMinX());
                minY = std::min(minY, beds[i].getMinY());
                maxX = std::max(maxX, beds[i].getMaxX());
                maxY = std::max(maxY, beds[i].getMaxY());
            }
            _bedRect = Rect(minX, minY, maxX - minX, maxY - minY);
        } else {
            float bedW = 120.0f, bedH = 60.0f;
            _bedRect = Rect(_roomRect.getMinX() + 24.0f,
                            _roomRect.getMaxY() - bedH - 24.0f,
                            bedW, bedH);
        }
    }

    _chestDraw = DrawNode::create();
    if (_roomMap && _roomMap->getTMX()) {
        _roomMap->getTMX()->addChild(_chestDraw, 19);
    } else if (_worldNode) {
        _worldNode->addChild(_chestDraw, 1);
    }
}

Size RoomMapController::getContentSize() const {
    if (_roomMap) return _roomMap->getContentSize();
    return Size(_roomRect.size.width, _roomRect.size.height);
}

Vec2 RoomMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    float pad = 16.0f;
    Vec2 candidate = next;
    candidate.x = std::max(_roomRect.getMinX() + pad, std::min(_roomRect.getMaxX() - pad, candidate.x));
    candidate.y = std::max(_roomRect.getMinY() + pad, std::min(_roomRect.getMaxY() - pad, candidate.y));
    // 基于对象层碰撞采样（脚下）
    if (_roomMap) {
        Vec2 footX(candidate.x, current.y);
        if (_roomMap->collides(footX + Vec2(0, -pad * 0.5f), radius)) {
            candidate.x = current.x;
        }
        Vec2 footY(current.x, candidate.y);
        if (_roomMap->collides(footY + Vec2(0, -pad * 0.5f), radius)) {
            candidate.y = current.y;
        }
    }
    for (const auto& ch : _chests) {
        if (Game::chestCollisionRect(ch).containsPoint(Vec2(candidate.x, candidate.y))) {
            return current;
        }
    }
    return candidate;
}

bool RoomMapController::isNearDoor(const Vec2& playerWorldPos) const {
    return _doorRect.containsPoint(playerWorldPos);
}

bool RoomMapController::isNearFarmDoor(const Vec2& playerWorldPos) const {
    return _roomMap ? _roomMap->nearDoorToFarm(playerWorldPos) : false;
}

bool RoomMapController::isNearChest(const Vec2& playerWorldPos) const {
    return Game::isNearAnyChest(playerWorldPos, _chests);
}

void RoomMapController::addActorToMap(cocos2d::Node* node, int zOrder) {
    if (_roomMap && _roomMap->getTMX()) {
        _roomMap->getTMX()->addChild(node, 20);
    } else if (_worldNode) {
        _worldNode->addChild(node, zOrder);
    }
}
bool RoomMapController::collides(const Vec2& pos, float radius) const {
    if (_roomMap && _roomMap->collides(pos, radius)) return true;
    for (const auto& ch : _chests) {
        if (Game::chestCollisionRect(ch).containsPoint(pos)) return true;
    }
    return false;
}
cocos2d::Vec2 RoomMapController::roomFarmDoorSpawnPos() const {
    if (_roomMap) {
		return _roomMap->doorToFarmCenter();
    }
    return cocos2d::Vec2(_doorRect.getMidX(), _doorRect.getMinY());
}

void RoomMapController::refreshChestsVisuals() {
    if (!_chestDraw) return;
    _chestDraw->clear();
    _chestDraw->removeAllChildren();
    for (const auto& ch : _chests) {
        auto r = Game::chestRect(ch);
        cocos2d::Vec2 center(r.getMidX(), r.getMidY());
        auto spr = cocos2d::Sprite::create("Chest.png");
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
            cocos2d::Vec2 a(r.getMinX(), r.getMinY());
            cocos2d::Vec2 b(r.getMaxX(), r.getMinY());
            cocos2d::Vec2 c(r.getMaxX(), r.getMaxY());
            cocos2d::Vec2 d(r.getMinX(), r.getMaxY());
            cocos2d::Vec2 v[4] = { a, b, c, d };
            _chestDraw->drawSolidPoly(v, 4, cocos2d::Color4F(0.6f,0.4f,0.2f,0.9f));
        }
    }
}

// namespace Controllers
}
