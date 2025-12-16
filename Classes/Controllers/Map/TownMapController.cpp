#include "Controllers/Map/TownMapController.h"
#include "Controllers/TileSelector.h"
#include "Game/GameConfig.h"
#include "Game/Chest.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace Controllers {

TownMapController::TownMapController(Game::TownMap* map, cocos2d::Node* worldNode)
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
    auto& ws = Game::globalState();
    _chests = ws.townChests;
    refreshMapVisuals();
}

Vec2 TownMapController::getPlayerPosition(const Vec2& playerMapLocalPos) const {
    if (!_worldNode) return playerMapLocalPos;
    Node* ref = (_map && _map->getTMX()) ? _map->getTMX() : _worldNode;
    if (!ref) return playerMapLocalPos;
    Vec2 world = ref->convertToWorldSpace(playerMapLocalPos);
    return _worldNode->convertToNodeSpace(world);
}

void TownMapController::addActorToMap(Node* node, int zOrder) {
    if (_map && _map->getTMX()) {
        _map->getTMX()->addChild(node, 20);
    } else if (_worldNode) {
        _worldNode->addChild(node, zOrder);
    }
}

Vec2 TownMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
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
    return finalPos;
}

std::pair<int,int> TownMapController::targetTile(const Vec2& playerPos, const Vec2& lastDir) const {
    return TileSelector::selectForwardTile(
        playerPos,
        lastDir,
        [this](const Vec2& p, int& c, int& r){ worldToTileIndex(p, c, r); },
        [this](int c, int r){ return inBounds(c, r); },
        _hasLastClick,
        _lastClickWorldPos,
        [this](int c, int r){ return tileToWorld(c, r); });
}

void TownMapController::updateCursor(const Vec2& playerPos, const Vec2& lastDir) {
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

bool TownMapController::collides(const Vec2& p, float radius) const {
    if (_map && _map->collides(p, radius)) return true;
    for (const auto& ch : _chests) {
        if (Game::chestCollisionRect(ch).containsPoint(p)) {
            return true;
        }
    }
    return false;
}

bool TownMapController::isNearChest(const Vec2& p) const {
    return Game::isNearAnyChest(p, _chests);
}

void TownMapController::refreshMapVisuals() {
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

void TownMapController::refreshDropsVisuals() {
    cocos2d::Node* parent = nullptr;
    if (_map && _map->getTMX()) {
        parent = _map->getTMX();
    } else if (_worldNode) {
        parent = _worldNode;
    }
    if (parent) {
        if (!_dropsDraw) {
            _dropsDraw = DrawNode::create();
            parent->addChild(_dropsDraw, 19);
        } else if (!_dropsDraw->getParent()) {
            _dropsDraw->removeFromParent();
            _dropsDraw = DrawNode::create();
            parent->addChild(_dropsDraw, 19);
        }
        if (!_dropsRoot) {
            _dropsRoot = Node::create();
            parent->addChild(_dropsRoot, 19);
        } else if (!_dropsRoot->getParent()) {
            _dropsRoot->removeFromParent();
            _dropsRoot = Node::create();
            parent->addChild(_dropsRoot, 19);
        }
    }
    Game::Drop::renderDrops(_drops, _dropsRoot, _dropsDraw);
}

void TownMapController::spawnDropAt(int c, int r, int itemType, int qty) {
    if (!_map || qty <= 0) return;
    if (!inBounds(c, r)) return;
    Game::Drop d{ static_cast<Game::ItemType>(itemType), tileToWorld(c, r), qty };
    _drops.push_back(d);
}

void TownMapController::collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv) {
    if (!inv) return;
    Game::Drop::collectDropsNear(playerWorldPos, _drops, inv);
    refreshDropsVisuals();
}

} // namespace Controllers
