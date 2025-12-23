#include "Controllers/Map/BeachMapController.h"
#include "Controllers/Interact/TileSelector.h"
#include "Game/GameConfig.h"
#include "Game/PlaceableItem/Chest.h"
#include "Game/WorldState.h"
#include "Controllers/Systems/FurnaceController.h"

using namespace cocos2d;

namespace Controllers {

BeachMapController::BeachMapController(Game::BeachMap* map, cocos2d::Node* worldNode)
: _map(map), _worldNode(worldNode) {
    if (_worldNode && _map) {
        cocos2d::Size content = _map->getContentSize();
        auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
        auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
        _origin = cocos2d::Vec2(origin.x + (visibleSize.width - content.width) * 0.5f,
                                origin.y + (visibleSize.height - content.height) * 0.5f);
        _map->setAnchorPoint(cocos2d::Vec2(0,0));
        _map->setPosition(_origin);
        _worldNode->addChild(_map, 0);
    }
    if (_map) {
        _cols = static_cast<int>(_map->getMapSize().width);
        _rows = static_cast<int>(_map->getMapSize().height);
        _tiles.assign(static_cast<size_t>(_cols) * static_cast<size_t>(_rows), Game::TileType::NotSoil);
    }
    auto& ws = Game::globalState();
    _chests = ws.beachChests;
    refreshMapVisuals();

    if (!_furnaceController) {
        _furnaceController = new Controllers::FurnaceController();
    }
    cocos2d::Node* furnaceParent = nullptr;
    if (_map && _map->getTMX()) {
        furnaceParent = _map->getTMX();
    } else if (_worldNode) {
        furnaceParent = _worldNode;
    }
    if (_furnaceController && furnaceParent) {
        _furnaceController->attachTo(furnaceParent, 19);
    }

    _dropSystem.configureTargetProvider([this]() -> Controllers::DropSystem::AttachTarget {
        Controllers::DropSystem::AttachTarget tgt;
        if (_map && _map->getTMX()) {
            tgt.parent = _map->getTMX();
            tgt.zOrder = 19;
        } else if (_worldNode) {
            tgt.parent = _worldNode;
            tgt.zOrder = 19;
        }
        return tgt;
    });
}

void BeachMapController::setFestivalActive(bool active) {
    if (_map) _map->setFestivalActive(active);
}

Vec2 BeachMapController::getPlayerPosition(const Vec2& playerMapLocalPos) const {
    if (!_worldNode) return playerMapLocalPos;
    Node* ref = (_map && _map->getTMX()) ? _map->getTMX() : _worldNode;
    if (!ref) return playerMapLocalPos;
    Vec2 world = ref->convertToWorldSpace(playerMapLocalPos);
    return _worldNode->convertToNodeSpace(world);
}

void BeachMapController::addActorToMap(Node* node, int zOrder) {
    if (_map && _map->getTMX()) {
        _map->getTMX()->addChild(node, 20);
    } else if (_worldNode) {
        _worldNode->addChild(node, zOrder);
    }
}

void BeachMapController::worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const {
    if (_map) { _map->worldToTileIndex(p, c, r); return; }
    c = 0;
    r = 0;
}

Vec2 BeachMapController::tileToWorld(int c, int r) const {
    return _map ? _map->tileToWorld(c, r) : cocos2d::Vec2();
}

bool BeachMapController::inBounds(int c, int r) const {
    return _map && c >= 0 && r >= 0 && c < static_cast<int>(_map->getMapSize().width) && r < static_cast<int>(_map->getMapSize().height);
}

Vec2 BeachMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    if (!_map) return next;
    float s = tileSize();
    Size ms = _map->getContentSize();
    float minX = s * 0.5f;
    float minY = s * 0.5f;
    float maxX = ms.width - s * 0.5f;
    float maxY = ms.height - s * 0.5f;
    Vec2 candidate(
        std::max(minX, std::min(maxX, next.x)),
        std::max(minY, std::min(maxY, next.y)));

    Vec2 tryX(candidate.x, current.y);
    Vec2 footX = tryX + Vec2(0, -s * 0.5f);
    if (_map->collides(footX, radius)) {
        tryX.x = current.x;
    }

    Vec2 tryY(current.x, candidate.y);
    Vec2 footY = tryY + Vec2(0, -s * 0.5f);
    if (_map->collides(footY, radius)) {
        tryY.y = current.y;
    }

    Vec2 finalPos(tryX.x, tryY.y);
    for (const auto& ch : _chests) {
        if (Game::chestCollisionRect(ch).containsPoint(finalPos)) {
            return current;
        }
    }
    if (_furnaceController && _furnaceController->collides(finalPos)) {
        return current;
    }
    return finalPos;
}

std::pair<int,int> BeachMapController::targetTile(const Vec2& playerPos, const Vec2& lastDir) const {
    return TileSelector::selectForwardTile(
        playerPos,
        lastDir,
        [this](const Vec2& p, int& c, int& r){ worldToTileIndex(p, c, r); },
        [this](int c, int r){ return inBounds(c, r); },
        tileSize(),
        _hasLastClick,
        _lastClickWorldPos,
        [this](int c, int r){ return tileToWorld(c, r); });
}

void BeachMapController::updateCursor(const Vec2& playerPos, const Vec2& lastDir) {
    if (!_cursor) {
        _cursor = DrawNode::create();
        if (_map && _map->getTMX()) {
            _map->getTMX()->addChild(_cursor, 19);
        } else if (_worldNode) {
            _worldNode->addChild(_cursor, 19);
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

bool BeachMapController::collides(const Vec2& p, float radius) const {
    if (_map && _map->collides(p, radius)) return true;
    for (const auto& ch : _chests) {
        if (Game::chestCollisionRect(ch).containsPoint(p)) {
            return true;
        }
    }
    if (_furnaceController && _furnaceController->collides(p)) {
        return true;
    }
    return false;
}

bool BeachMapController::isNearDoor(const Vec2& p) const {
    return _map ? _map->nearDoorToFarm(p) : false;
}

Game::TileType BeachMapController::getTile(int c, int r) const {
    if (c < 0 || r < 0 || c >= _cols || r >= _rows) return Game::TileType::NotSoil;
    size_t idx = static_cast<size_t>(r) * static_cast<size_t>(_cols) + static_cast<size_t>(c);
    if (idx < _tiles.size()) return _tiles[idx];
    return Game::TileType::NotSoil;
}

void BeachMapController::setTile(int c, int r, Game::TileType) {
    if (c < 0 || r < 0 || c >= _cols || r >= _rows) return;
    size_t idx = static_cast<size_t>(r) * static_cast<size_t>(_cols) + static_cast<size_t>(c);
    if (idx < _tiles.size()) {
        _tiles[idx] = Game::TileType::NotSoil;
    }
}

void BeachMapController::setLastClickWorldPos(const cocos2d::Vec2& p) {
    _lastClickWorldPos = p;
    _hasLastClick = true;
}

void BeachMapController::refreshMapVisuals() {
    if (!_chestDraw) {
        _chestDraw = DrawNode::create();
        if (_map && _map->getTMX()) {
            _map->getTMX()->addChild(_chestDraw, 19);
        } else if (_worldNode) {
            _worldNode->addChild(_chestDraw, 19);
        }
    }
    if (!_chestDraw) return;
    _chestDraw->clear();
    _chestDraw->removeAllChildren();
    for (const auto& ch : _chests) {
        auto r = Game::chestRect(ch);
        Vec2 center(r.getMidX(), r.getMidY());
        auto spr = Sprite::create("Chest.png");
        if (spr && spr->getTexture()) {
            auto cs = spr->getContentSize();
            if (cs.width > 0.0f && cs.height > 0.0f) {
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
            Vec2 c2(r.getMaxX(), r.getMaxY());
            Vec2 d(r.getMinX(), r.getMaxY());
            Vec2 v[4] = { a, b, c2, d };
            _chestDraw->drawSolidPoly(v, 4, Color4F(0.6f,0.4f,0.2f,0.9f));
        }
    }
}

void BeachMapController::refreshDropsVisuals() {
    _dropSystem.refreshVisuals();
}

void BeachMapController::spawnDropAt(int c, int r, int itemType, int qty) {
    _dropSystem.spawnDropAt(this, c, r, itemType, qty);
}

void BeachMapController::collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv) {
    _dropSystem.collectDropsNear(playerWorldPos, inv);
}

} // namespace Controllers
