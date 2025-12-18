#include "Controllers/Map/RoomMapController.h"
#include "cocos2d.h"
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
#include "Game/Map/RoomMap.h"
#include "Controllers/Systems/FurnaceController.h"

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

    if (!_chestController) {
        _chestController = new Controllers::ChestController(false);
    }
    cocos2d::Node* chestParent = nullptr;
    if (_roomMap && _roomMap->getTMX()) {
        chestParent = _roomMap->getTMX();
    } else if (_worldNode) {
        chestParent = _worldNode;
    }
    if (_chestController && chestParent) {
        _chestController->attachTo(chestParent, 19);
        _chestController->syncLoad();
    }

    if (!_furnaceController) {
        _furnaceController = new Controllers::FurnaceController();
    }
    cocos2d::Node* furnaceParent = nullptr;
    if (_roomMap && _roomMap->getTMX()) {
        furnaceParent = _roomMap->getTMX();
    } else if (_worldNode) {
        furnaceParent = _worldNode;
    }
    if (_furnaceController && furnaceParent) {
        _furnaceController->attachTo(furnaceParent, 19);
    }
}

Vec2 RoomMapController::getPlayerPosition(const Vec2& playerMapLocalPos) const {
    if (!_worldNode) return playerMapLocalPos;
    Node* ref = (_roomMap && _roomMap->getTMX()) ? _roomMap->getTMX() : _worldNode;
    if (!ref) return playerMapLocalPos;
    Vec2 world = ref->convertToWorldSpace(playerMapLocalPos);
    return _worldNode->convertToNodeSpace(world);
}

Size RoomMapController::getContentSize() const {
    if (_roomMap) return _roomMap->getContentSize();
    return Size(_roomRect.size.width, _roomRect.size.height);
}

Vec2 RoomMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    Vec2 candidate = next;
    candidate.x = std::max(_roomRect.getMinX() + s * 0.5f,
                           std::min(_roomRect.getMaxX() - s * 0.5f, candidate.x));
    candidate.y = std::max(_roomRect.getMinY() + s * 0.5f,
                           std::min(_roomRect.getMaxY() - s * 0.5f, candidate.y));

    Vec2 tryX(candidate.x, current.y);
    if (_roomMap) {
        Vec2 footX = tryX + Vec2(0, -s * 0.5f);
        if (_roomMap->collides(footX, radius)) {
            tryX.x = current.x;
        }
    }

    Vec2 tryY(current.x, candidate.y);
    if (_roomMap) {
        Vec2 footY = tryY + Vec2(0, -s * 0.5f);
        if (_roomMap->collides(footY, radius)) {
            tryY.y = current.y;
        }
    }

    Vec2 finalPos(tryX.x, tryY.y);
    if (_chestController && _chestController->collides(finalPos)) {
        return current;
    }
    return finalPos;
}

bool RoomMapController::isNearDoor(const Vec2& playerWorldPos) const {
    return _doorRect.containsPoint(playerWorldPos);
}

bool RoomMapController::isNearFarmDoor(const Vec2& playerWorldPos) const {
    return _roomMap ? _roomMap->nearDoorToFarm(playerWorldPos) : false;
}

bool RoomMapController::isNearChest(const Vec2& playerWorldPos) const {
    return _chestController ? _chestController->isNearChest(playerWorldPos) : false;
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
    if (_chestController && _chestController->collides(pos)) return true;
    return false;
}
cocos2d::Vec2 RoomMapController::roomFarmDoorSpawnPos() const {
    if (_roomMap) {
		return _roomMap->doorToFarmCenter();
    }
    return cocos2d::Vec2(_doorRect.getMidX(), _doorRect.getMinY());
}

void RoomMapController::refreshChestsVisuals() {
    if (_chestController) {
        _chestController->refreshVisuals();
    }
}

// namespace Controllers
}
