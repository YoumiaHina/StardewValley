#include "Controllers/Map/FarmMapController.h"
#include "cocos2d.h"
#include <random>
#include <ctime>
#include <algorithm>
#include <unordered_set>
#include "Game/GameConfig.h"
#include "Game/WorldState.h"
#include "Game/Crops/crop/CropBase.h"

using namespace cocos2d;

namespace Controllers {

FarmMapController::FarmMapController(cocos2d::Node* worldNode)
: _worldNode(worldNode) {}

EnvironmentObstacleSystemBase* FarmMapController::obstacleSystem(ObstacleKind kind) {
    if (kind == ObstacleKind::Tree) return _treeSystem;
    if (kind == ObstacleKind::Rock) return _rockSystem;
    return nullptr;
}

const EnvironmentObstacleSystemBase* FarmMapController::obstacleSystem(ObstacleKind kind) const {
    if (kind == ObstacleKind::Tree) return _treeSystem;
    if (kind == ObstacleKind::Rock) return _rockSystem;
    return nullptr;
}

const std::vector<Game::Chest>& FarmMapController::chests() const {
    static const std::vector<Game::Chest> empty;
    return _chestController ? _chestController->chests() : empty;
}

std::vector<Game::Chest>& FarmMapController::chests() {
    static std::vector<Game::Chest> empty;
    return _chestController ? _chestController->chests() : empty;
}

void FarmMapController::setLastClickWorldPos(const cocos2d::Vec2& p) {
    _lastClickWorldPos = p;
    _hasLastClick = true;
}

void FarmMapController::init() {
    if (!_worldNode) return;
    _mapNode = Node::create();
    _worldNode->addChild(_mapNode, 0);

    _farmMap = Game::FarmMap::create("Maps/spring_outdoors/spring_outdoors.tmx");
    if (!_farmMap) return;
    Size content = _farmMap->getContentSize();
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();
    _mapOrigin = Vec2(origin.x + (visibleSize.width - content.width) * 0.5f,
                      origin.y + (visibleSize.height - content.height) * 0.5f);
    _farmMap->setAnchorPoint(Vec2(0,0));
    _farmMap->setPosition(_mapOrigin);
    _mapNode->addChild(_farmMap, 0);
    _cols = static_cast<int>(_farmMap->getMapSize().width);
    _rows = static_cast<int>(_farmMap->getMapSize().height);

    auto &ws = Game::globalState();
    if (ws.farmTiles.empty()) {
        _tiles.assign(_cols * _rows, Game::TileType::Soil);
        applyStaticNotSoilMask();
        int cx = _cols / 2;
        int cy = _rows / 2;
        auto safe = [cx, cy](int c, int r){ return std::abs(c - cx) <= 4 && std::abs(r - cy) <= 4; };
        std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::uniform_int_distribution<int> distC(0, _cols - 1);
        std::uniform_int_distribution<int> distR(0, _rows - 1);
        auto placeIfSoil = [this, &safe](int c, int r, Game::TileType t){
            if (!inBounds(c,r)) return;
            if (safe(c,r)) return;
            if (getTile(c,r) != Game::TileType::Soil) return;
            if (t == Game::TileType::Tree && _farmMap) {
                auto center = this->tileToWorld(c, r);
                cocos2d::Vec2 footCenter = center + cocos2d::Vec2(0, -static_cast<float>(GameConfig::TILE_SIZE) * 0.5f);
                if (_farmMap->inBuildingArea(footCenter)) return;
            }
            setTile(c, r, t);
        };
        int rocks = (_cols * _rows) / 22;
        int trees = (_cols * _rows) / 40;
        for (int i = 0; i < rocks; ++i) { placeIfSoil(distC(rng), distR(rng), Game::TileType::Rock); }
        for (int i = 0; i < trees; ++i) { placeIfSoil(distC(rng), distR(rng), Game::TileType::Tree); }
        ws.farmTiles = _tiles;
        ws.farmCols = _cols;
        ws.farmRows = _rows;
    } else {
        // 同步已有瓦片
        _tiles = ws.farmTiles;
        // 如果 WorldState 中未保存尺寸（旧存档兼容），则补全
        if (ws.farmCols == 0) ws.farmCols = _cols;
        if (ws.farmRows == 0) ws.farmRows = _rows;
        applyStaticNotSoilMask();
    }

    _cursor = DrawNode::create();
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(_cursor, 21);
    } else {
        _mapNode->addChild(_cursor, 1);
    }

    _dropSystem.configureTargetProvider([this]() -> Controllers::DropSystem::AttachTarget {
        Controllers::DropSystem::AttachTarget tgt;
        if (_farmMap && _farmMap->getTMX()) {
            tgt.parent = _farmMap->getTMX();
            tgt.zOrder = 19;
        } else if (_worldNode) {
            tgt.parent = _worldNode;
            tgt.zOrder = 1;
        }
        return tgt;
    });
    _dropSystem.setOnDropsChanged([](const std::vector<Game::Drop>& drops) {
        Game::globalState().farmDrops = drops;
    });
    _dropSystem.setDrops(ws.farmDrops);
    if (!_chestController) {
        _chestController = new Controllers::ChestController(true);
    }
    if (_chestController) {
        Node* chestParent = nullptr;
        if (_farmMap && _farmMap->getTMX()) {
            chestParent = _farmMap->getTMX();
        } else if (_worldNode) {
            chestParent = _worldNode;
        }
        if (chestParent) {
            _chestController->attachTo(chestParent, 19);
            _chestController->syncLoad();
        }
    }
    if (!_furnaceController) {
        _furnaceController = new Controllers::FurnaceController();
    }
    if (_furnaceController) {
        Node* furnaceParent = nullptr;
        if (_farmMap && _farmMap->getTMX()) {
            furnaceParent = _farmMap->getTMX();
        } else if (_worldNode) {
            furnaceParent = _worldNode;
        }
        if (furnaceParent) {
            _furnaceController->attachTo(furnaceParent, 19);
        }
    }
    _cropsDraw = DrawNode::create();
    _worldNode->addChild(_cropsDraw, 1);
    _cropsRoot = Node::create();
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(_cropsRoot, 22);
    } else {
        _worldNode->addChild(_cropsRoot, 18);
    }

    // Tile overlay root for textured tilled soil
    _tileRoot = Node::create();
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(_tileRoot, 18);
    } else {
        _worldNode->addChild(_tileRoot, 0);
    }

    _actorsRoot = Node::create();
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(_actorsRoot, 20);
    } else {
        _worldNode->addChild(_actorsRoot, 20);
    }

    _treeSystem = new Controllers::TreeSystem();
    _treeSystem->attachTo(_actorsRoot);
    _rockSystem = new Controllers::RockSystem();
    _rockSystem->attachTo(_actorsRoot);

    refreshMapVisuals();
    refreshDropsVisuals();
    refreshCropsVisuals();

    if (_treeSystem && _treeSystem->isEmpty()) {
        if (!ws.farmTrees.empty()) {
            auto treeSystemConcrete = static_cast<Controllers::TreeSystem*>(_treeSystem);
            for (const auto& tp : ws.farmTrees) {
                auto center = tileToWorld(tp.c, tp.r);
                if (treeSystemConcrete) {
                    treeSystemConcrete->spawnFromTileWithKind(tp.c, tp.r, center, _farmMap, GameConfig::TILE_SIZE, tp.kind);
                } else {
                    _treeSystem->spawnFromTile(tp.c, tp.r, center, _farmMap, GameConfig::TILE_SIZE);
                }
            }
        } else {
            std::vector<Game::TreePos> saved;
            int created = 0;
            auto treeSystemConcrete = static_cast<Controllers::TreeSystem*>(_treeSystem);
            for (int r = 0; r < _rows; ++r) {
                for (int c = 0; c < _cols; ++c) {
                    if (getTile(c, r) == Game::TileType::Tree) {
                        auto center = tileToWorld(c, r);
                        Vec2 footCenter = center + Vec2(0, -static_cast<float>(GameConfig::TILE_SIZE) * 0.5f);
                        bool skip = _farmMap && _farmMap->inBuildingArea(footCenter);
                        if (skip) continue;
                        if (_treeSystem->spawnFromTile(c, r, center, _farmMap, GameConfig::TILE_SIZE)) {
                            setTile(c, r, Game::TileType::Soil);
                            Game::TreeKind kind = Game::TreeKind::Tree1;
                            if (treeSystemConcrete) {
                                auto tree = treeSystemConcrete->findTreeAt(c, r);
                                if (tree) kind = tree->kind();
                            }
                            saved.push_back(Game::TreePos{c, r, kind});
                            created++;
                        }
                    }
                }
            }
            if (created == 0) {
                int cx = _cols / 2;
                int cy = _rows / 2;
                auto safe = [this, cx, cy](int c, int r){
                    if (std::abs(c - cx) <= 4 && std::abs(r - cy) <= 4) return true;
                    if (!inBounds(c, r)) return true;
                    auto t = getTile(c, r);
                    if (t == Game::TileType::NotSoil) return true;
                    if (_farmMap) {
                        auto center = this->tileToWorld(c, r);
                        Vec2 footCenter = center + Vec2(0, -static_cast<float>(GameConfig::TILE_SIZE) * 0.5f);
                        if (_farmMap->inBuildingArea(footCenter)) return true;
                    }
                    return false;
                };
                int trees = (_cols * _rows) / 40;
                _treeSystem->spawnRandom(trees, _cols, _rows,
                    [this](int c, int r){ return this->tileToWorld(c, r); },
                    _farmMap, GameConfig::TILE_SIZE,
                    [safe](int c, int r){ return safe(c, r); }
                );
                if (treeSystemConcrete) {
                    saved = treeSystemConcrete->getAllTreeTiles();
                }
            }
            ws.farmTrees = saved;
        }
    }

    if (_rockSystem && _rockSystem->isEmpty()) {
        if (!ws.farmRocks.empty()) {
            auto rockSystemConcrete = static_cast<Controllers::RockSystem*>(_rockSystem);
            for (const auto& rp : ws.farmRocks) {
                auto center = tileToWorld(rp.c, rp.r);
                if (rockSystemConcrete) {
                    rockSystemConcrete->spawnFromTileWithKind(rp.c, rp.r, center, _farmMap, GameConfig::TILE_SIZE, rp.kind);
                } else {
                    _rockSystem->spawnFromTile(rp.c, rp.r, center, _farmMap, GameConfig::TILE_SIZE);
                }
            }
        } else {
            std::vector<Game::RockPos> savedRocks;
            auto rockSystemConcrete = static_cast<Controllers::RockSystem*>(_rockSystem);
            for (int r = 0; r < _rows; ++r) {
                for (int c = 0; c < _cols; ++c) {
                    if (getTile(c, r) == Game::TileType::Rock) {
                        auto center = tileToWorld(c, r);
                        if (_rockSystem->spawnFromTile(c, r, center, _farmMap, GameConfig::TILE_SIZE)) {
                            setTile(c, r, Game::TileType::Soil);
                            Game::RockKind kind = Game::RockKind::Rock1;
                            if (rockSystemConcrete) {
                                auto rock = rockSystemConcrete->findRockAt(c, r);
                                if (rock) kind = rock->kind();
                            }
                            savedRocks.push_back(Game::RockPos{c, r, kind});
                        }
                    }
                }
            }
            ws.farmRocks = savedRocks;
        }
    }

    // 门口区域
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float doorW = s * 2.0f;
    float doorH = s * 0.75f;
    float dx = (_cols * s) * 0.5f - doorW * 0.5f;
    float dy = s * 0.5f - doorH * 0.5f;
    _farmDoorRect = Rect(dx, dy, doorW, doorH);
}

void FarmMapController::applyStaticNotSoilMask() {
    if (!_farmMap) return;
    float s = tileSize();
    bool changed = false;
    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _cols; ++c) {
            auto current = _tiles[r * _cols + c];
            if (current != Game::TileType::Soil &&
                current != Game::TileType::Tilled &&
                current != Game::TileType::Watered) {
                continue;
            }
            auto center = tileToWorld(c, r);
            Vec2 footCenter = center + Vec2(0, -s * 0.5f);
            if (_farmMap->inBuildingArea(footCenter) || _farmMap->inWallArea(footCenter)) {
                _tiles[r * _cols + c] = Game::TileType::NotSoil;
                changed = true;
            }
        }
    }
    if (changed) {
        Game::globalState().farmTiles = _tiles;
    }
}

Vec2 FarmMapController::getPlayerPosition(const Vec2& playerMapLocalPos) const {
    if (!_worldNode) return playerMapLocalPos;
    Node* ref = _actorsRoot ? _actorsRoot : (_farmMap && _farmMap->getTMX() ? _farmMap->getTMX() : _worldNode);
    if (!ref) return playerMapLocalPos;
    Vec2 world = ref->convertToWorldSpace(playerMapLocalPos);
    return _worldNode->convertToNodeSpace(world);
}

Size FarmMapController::getContentSize() const {
    if (_farmMap) return _farmMap->getContentSize();
    return Size(_cols * GameConfig::TILE_SIZE, _rows * GameConfig::TILE_SIZE);
}

Vec2 FarmMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float mapW = _farmMap ? _farmMap->getContentSize().width : (_cols * s);
    float mapH = _farmMap ? _farmMap->getContentSize().height : (_rows * s);
    Vec2 candidate = next;
    float minX = _farmMap ? (s * 0.5f) : (_mapOrigin.x + s * 0.5f);
    float minY = _farmMap ? (s * 0.5f) : (_mapOrigin.y + s * 0.5f);
    float maxX = _farmMap ? (mapW - s * 0.5f) : (_mapOrigin.x + mapW - s * 0.5f);
    float maxY = _farmMap ? (mapH - s * 0.5f) : (_mapOrigin.y + mapH - s * 0.5f);
    candidate.x = std::max(minX, std::min(maxX, candidate.x));
    candidate.y = std::max(minY, std::min(maxY, candidate.y));

    Vec2 tryX(candidate.x, current.y);
    if (_farmMap) {
        Vec2 footX = tryX + Vec2(0, -s * 0.5f);
        bool baseBlockedX = _farmMap->collides(footX, radius);
        bool treeBlockedX = false;
        bool rockBlockedX = false;
        if (_treeSystem) {
            treeBlockedX = _treeSystem->collides(footX, radius * 0.75f, GameConfig::TILE_SIZE);
        }
        if (_rockSystem) {
            rockBlockedX = _rockSystem->collides(footX, radius * 0.75f, GameConfig::TILE_SIZE);
        }
        if (baseBlockedX || treeBlockedX || rockBlockedX) {
            tryX.x = current.x;
        }
    }
    Vec2 tryY(current.x, candidate.y);
    if (_farmMap) {
        Vec2 footY = tryY + Vec2(0, -s * 0.5f);
        bool baseBlockedY = _farmMap->collides(footY, radius);
        bool treeBlockedY = false;
        bool rockBlockedY = false;
        if (_treeSystem) {
            treeBlockedY = _treeSystem->collides(footY, radius * 0.75f, GameConfig::TILE_SIZE);
        }
        if (_rockSystem) {
            rockBlockedY = _rockSystem->collides(footY, radius * 0.75f, GameConfig::TILE_SIZE);
        }
        if (baseBlockedY || treeBlockedY || rockBlockedY) {
            tryY.y = current.y;
        }
    }
    Vec2 testPos(tryX.x, tryY.y);
    if ((_chestController && _chestController->collides(testPos)) ||
        (_furnaceController && _furnaceController->collides(testPos))) {
        return current;
    }
    return Vec2(tryX.x, tryY.y);
}

bool FarmMapController::collides(const Vec2& pos, float radius) const {
    if (_farmMap && _farmMap->collides(pos, radius)) return true;
    if (_treeSystem && _treeSystem->collides(pos, radius, GameConfig::TILE_SIZE)) return true;
    if (_rockSystem && _rockSystem->collides(pos, radius, GameConfig::TILE_SIZE)) return true;
    if (_chestController && _chestController->collides(pos)) return true;
    if (_furnaceController && _furnaceController->collides(pos)) return true;
    return false;
}

bool FarmMapController::isNearDoor(const Vec2& playerWorldPos) const {
    if (_farmMap) {
        return _farmMap->nearDoorToRoom(playerWorldPos);
    }
    return _farmDoorRect.containsPoint(playerWorldPos - _mapOrigin);
}

bool FarmMapController::isNearMineDoor(const Vec2& playerWorldPos) const {
    return _farmMap ? _farmMap->nearDoorToMine(playerWorldPos) : false;
}

bool FarmMapController::isNearBeachDoor(const Vec2& playerWorldPos) const {
    return _farmMap ? _farmMap->nearDoorToBeach(playerWorldPos) : false;
}

bool FarmMapController::isNearTownDoor(const Vec2& playerWorldPos) const {
    return _farmMap ? _farmMap->nearDoorToTown(playerWorldPos) : false;
}

bool FarmMapController::isNearChest(const Vec2& playerWorldPos) const {
    return _chestController ? _chestController->isNearChest(playerWorldPos) : false;
}

bool FarmMapController::isNearLake(const Vec2& playerWorldPos, float radius) const {
    return _farmMap ? _farmMap->nearWater(playerWorldPos, radius) : false;
}

void FarmMapController::sortActorWithEnvironment(cocos2d::Node* actor) {
    if (!actor) return;
    float s = tileSize();
    float footY = actor->getPositionY() - s * 0.5f; // player node is at tile center; use foot for sorting
    actor->setLocalZOrder(static_cast<int>(-footY));
    auto treeSystemConcrete = static_cast<Controllers::TreeSystem*>(_treeSystem);
    if (treeSystemConcrete) treeSystemConcrete->sortTrees();
    auto rockSystemConcrete = static_cast<Controllers::RockSystem*>(_rockSystem);
    if (rockSystemConcrete) rockSystemConcrete->sortRocks();
}

cocos2d::Vec2 FarmMapController::farmMineDoorSpawnPos() const {
    if (_farmMap) {
        return _farmMap->doorToMineCenter();
    }
    // fallback：使用农场门矩形中心
    return Vec2(_farmDoorRect.getMidX(), _farmDoorRect.getMidY());
}

cocos2d::Vec2 FarmMapController::farmRoomDoorSpawnPos() const {
    if (_farmMap) {
        return _farmMap->doorToRoomCenter();
    }
    return Vec2(_farmDoorRect.getMidX(), _farmDoorRect.getMidY());
}

cocos2d::Vec2 FarmMapController::farmBeachDoorSpawnPos() const {
    if (_farmMap) {
        return _farmMap->doorToBeachCenter();
    }
    return Vec2(_farmDoorRect.getMidX(), _farmDoorRect.getMidY());
}

cocos2d::Vec2 FarmMapController::farmTownDoorSpawnPos() const {
    if (_farmMap) {
        return _farmMap->doorToTownCenter();
    }
    return Vec2(_farmDoorRect.getMidX(), _farmDoorRect.getMidY());
}

bool FarmMapController::inBounds(int c, int r) const {
    return c >= 0 && r >= 0 && c < _cols && r < _rows;
}

std::pair<int,int> FarmMapController::targetTile(const Vec2& playerPos, const Vec2& lastDir) const {
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


void FarmMapController::updateCursor(const Vec2& playerPos, const Vec2& lastDir) {
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

Game::TileType FarmMapController::getTile(int c, int r) const {
    auto t = _tiles[r * _cols + c];
    return t;
}

void FarmMapController::setTile(int c, int r, Game::TileType t) {
    _tiles[r * _cols + c] = t;
    Game::globalState().farmTiles = _tiles;
    // Immediately refresh visuals to reflect tile changes
    refreshMapVisuals();
}

Vec2 FarmMapController::tileToWorld(int c, int r) const {
    if (_farmMap) return _farmMap->tileToWorld(c, r);
    float s = tileSize();
    return _mapOrigin + Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
}

void FarmMapController::worldToTileIndex(const Vec2& p, int& c, int& r) const {
    if (_farmMap) { _farmMap->worldToTileIndex(p, c, r); return; }
    float s = tileSize();
    c = static_cast<int>((p.x - _mapOrigin.x) / s);
    r = static_cast<int>((p.y - _mapOrigin.y) / s);
}

void FarmMapController::refreshMapVisuals() {
    if (!_tileRoot) return;
    float s = tileSize();
    std::unordered_set<long long> alive;
    std::unordered_set<long long> aliveWater;
    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _cols; ++c) {
            if (getTile(c, r) == Game::TileType::Tilled || getTile(c, r) == Game::TileType::Watered) {
                long long key = (static_cast<long long>(r) << 32) | static_cast<unsigned long long>(c);
                alive.insert(key);
                cocos2d::Sprite* spr = nullptr;
                auto it = _tileSprites.find(key);
                if (it == _tileSprites.end()) {
                    spr = cocos2d::Sprite::create("hoeDirt.png");
                    spr->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
                    _tileRoot->addChild(spr, 0);
                    _tileSprites[key] = spr;
                } else {
                    spr = it->second;
                }
                int tw = 16, th = 16;
                bool up = false, down = false, left = false, right = false;
                if (r + 1 < _rows) {
                    auto t = getTile(c, r + 1);
                    up = (t == Game::TileType::Tilled || t == Game::TileType::Watered);
                }
                if (r - 1 >= 0) {
                    auto t = getTile(c, r - 1);
                    down = (t == Game::TileType::Tilled || t == Game::TileType::Watered);
                }
                if (c - 1 >= 0) {
                    auto t = getTile(c - 1, r);
                    left = (t == Game::TileType::Tilled || t == Game::TileType::Watered);
                }
                if (c + 1 < _cols) {
                    auto t = getTile(c + 1, r);
                    right = (t == Game::TileType::Tilled || t == Game::TileType::Watered);
                }
                int mask = 0;
                if (up) mask |= 1;
                if (down) mask |= 2;
                if (left) mask |= 4;
                if (right) mask |= 8;
                int rowBottom = 4;
                int colLeft = 1;
                switch (mask) {
                    case 0:  rowBottom = 1; colLeft = 1; break;
                    case 2:  rowBottom = 2; colLeft = 1; break;
                    case 1:  rowBottom = 4; colLeft = 1; break;
                    case 3:  rowBottom = 3; colLeft = 1; break;
                    case 8:  rowBottom = 4; colLeft = 2; break;
                    case 4:  rowBottom = 4; colLeft = 4; break;
                    case 12: rowBottom = 4; colLeft = 3; break;
                    case 10: rowBottom = 1; colLeft = 2; break;
                    case 6:  rowBottom = 1; colLeft = 4; break;
                    case 5:  rowBottom = 3; colLeft = 4; break;
                    case 9:  rowBottom = 3; colLeft = 2; break;
                    case 11: rowBottom = 2; colLeft = 2; break;
                    case 13: rowBottom = 3; colLeft = 3; break;
                    case 14: rowBottom = 1; colLeft = 3; break;
                    case 7:  rowBottom = 2; colLeft = 4; break;
                    case 15: rowBottom = 2; colLeft = 3; break;
                    default: rowBottom = 1; colLeft = 1; break;
                }
                float texW = spr->getTexture() ? spr->getTexture()->getContentSize().width : 0.0f;
                float texH = spr->getTexture() ? spr->getTexture()->getContentSize().height : 0.0f;
                int totalRows = texH > 0 ? static_cast<int>(texH / th) : 1;
                int colIndex0 = colLeft - 1;
                int rowIndexFromTop0 = totalRows - rowBottom;
                if (rowIndexFromTop0 < 0) rowIndexFromTop0 = 0;
                float x = static_cast<float>(colIndex0 * tw);
                float y = texH - static_cast<float>((rowIndexFromTop0 + 1) * th);
                spr->setTextureRect(cocos2d::Rect(x, y, static_cast<float>(tw), static_cast<float>(th)));
                auto pos = tileToWorld(c, r);
                spr->setPosition(pos);
                spr->setVisible(true);

                if (getTile(c, r) == Game::TileType::Watered) {
                    aliveWater.insert(key);
                    cocos2d::Sprite* sprW = nullptr;
                    auto itW = _waterSprites.find(key);
                    if (itW == _waterSprites.end()) {
                        sprW = cocos2d::Sprite::create("hoeDirt.png");
                        sprW->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
                        _tileRoot->addChild(sprW, 1);
                        _waterSprites[key] = sprW;
                    } else {
                        sprW = itW->second;
                    }
                    bool upW = false, downW = false, leftW = false, rightW = false;
                    if (r + 1 < _rows) {
                        auto t = getTile(c, r + 1);
                        upW = (t == Game::TileType::Watered);
                    }
                    if (r - 1 >= 0) {
                        auto t = getTile(c, r - 1);
                        downW = (t == Game::TileType::Watered);
                    }
                    if (c - 1 >= 0) {
                        auto t = getTile(c - 1, r);
                        leftW = (t == Game::TileType::Watered);
                    }
                    if (c + 1 < _cols) {
                        auto t = getTile(c + 1, r);
                        rightW = (t == Game::TileType::Watered);
                    }
                    int maskW = 0;
                    if (upW) maskW |= 1;
                    if (downW) maskW |= 2;
                    if (leftW) maskW |= 4;
                    if (rightW) maskW |= 8;
                    int rowBottomW = 4;
                    int colLeftW = 1;
                    switch (maskW) {
                        case 0:  rowBottomW = 1; colLeftW = 1; break;
                        case 2:  rowBottomW = 2; colLeftW = 1; break;
                        case 1:  rowBottomW = 4; colLeftW = 1; break;
                        case 3:  rowBottomW = 3; colLeftW = 1; break;
                        case 8:  rowBottomW = 4; colLeftW = 2; break;
                        case 4:  rowBottomW = 4; colLeftW = 4; break;
                        case 12: rowBottomW = 4; colLeftW = 3; break;
                        case 10: rowBottomW = 1; colLeftW = 2; break;
                        case 6:  rowBottomW = 1; colLeftW = 4; break;
                        case 5:  rowBottomW = 3; colLeftW = 4; break;
                        case 9:  rowBottomW = 3; colLeftW = 2; break;
                        case 11: rowBottomW = 2; colLeftW = 2; break;
                        case 13: rowBottomW = 3; colLeftW = 3; break;
                        case 14: rowBottomW = 1; colLeftW = 3; break;
                        case 7:  rowBottomW = 2; colLeftW = 4; break;
                        case 15: rowBottomW = 2; colLeftW = 3; break;
                        default: rowBottomW = 1; colLeftW = 1; break;
                    }
                    colLeftW += 4;
                    float texWW = sprW->getTexture() ? sprW->getTexture()->getContentSize().width : 0.0f;
                    float texHW = sprW->getTexture() ? sprW->getTexture()->getContentSize().height : 0.0f;
                    int columnsW = texWW > 0 ? static_cast<int>(texWW / tw) : 1;
                    int totalRowsW = texHW > 0 ? static_cast<int>(texHW / th) : 1;
                    int colIndex0W = colLeftW - 1;
                    int rowIndexFromTop0W = totalRowsW - rowBottomW;
                    if (rowIndexFromTop0W < 0) rowIndexFromTop0W = 0;
                    float xW = static_cast<float>(colIndex0W * tw);
                    float yW = texHW - static_cast<float>((rowIndexFromTop0W + 1) * th);
                    sprW->setTextureRect(cocos2d::Rect(xW, yW, static_cast<float>(tw), static_cast<float>(th)));
                    auto posW = tileToWorld(c, r);
                    sprW->setPosition(posW);
                    sprW->setVisible(true);
                }
            }
        }
    }

    // Remove any overlay sprites that are no longer needed
    std::vector<long long> toRemove;
    for (auto &kv : _tileSprites) {
        if (alive.find(kv.first) == alive.end()) {
            toRemove.push_back(kv.first);
        }
    }
    for (auto k : toRemove) {
        if (_tileSprites.count(k)) {
            auto spr = _tileSprites[k];
            if (spr) spr->removeFromParent();
            _tileSprites.erase(k);
        }
    }
    std::vector<long long> toRemoveWater;
    for (auto &kv : _waterSprites) {
        if (aliveWater.find(kv.first) == aliveWater.end()) {
            toRemoveWater.push_back(kv.first);
        }
    }
    for (auto k : toRemoveWater) {
        if (_waterSprites.count(k)) {
            auto spr = _waterSprites[k];
            if (spr) spr->removeFromParent();
            _waterSprites.erase(k);
        }
    }
    if (_chestController) {
        _chestController->refreshVisuals();
    }
}

void FarmMapController::refreshCropsVisuals() {
    if (!_cropsRoot || !_cropsDraw) return;
    float s = tileSize();
    std::unordered_set<long long> alive;
    for (const auto& cp : Game::globalState().farmCrops) {
        long long key = (static_cast<long long>(cp.r) << 32) | static_cast<unsigned long long>(cp.c);
        alive.insert(key);

        cocos2d::Sprite* sprB = nullptr;
        cocos2d::Sprite* sprT = nullptr;

        auto itB = _cropSprites.find(key);
        if (itB == _cropSprites.end()) {
            sprB = cocos2d::Sprite::create("Crops/Crops.png");
            if (sprB) {
                sprB->setAnchorPoint(cocos2d::Vec2(0.5f, 0.0f));
                _cropsRoot->addChild(sprB, 0);
                _cropSprites[key] = sprB;
            }
        } else {
            sprB = itB->second;
        }

        auto itT = _cropSpritesTop.find(key);
        if (itT == _cropSpritesTop.end()) {
            sprT = cocos2d::Sprite::create("Crops/Crops.png");
            if (sprT) {
                sprT->setAnchorPoint(cocos2d::Vec2(0.5f, 0.0f));
                _cropsRoot->addChild(sprT, 1);
                _cropSpritesTop[key] = sprT;
            }
        } else {
            sprT = itT->second;
        }

        auto center = tileToWorld(cp.c, cp.r);
        if (sprB && sprT && sprB->getTexture()) {
            float texH = sprB->getTexture()->getContentSize().height;
            cocos2d::Rect rectB = Game::cropRectBottomHalf(cp.type, cp.stage, texH);
            cocos2d::Rect rectT = Game::cropRectTopHalf(cp.type, cp.stage, texH);
            sprB->setTextureRect(rectB);
            sprT->setTextureRect(rectT);
            sprB->setPosition(cocos2d::Vec2(center.x, center.y - s * 0.5f));
            sprT->setPosition(cocos2d::Vec2(center.x, center.y - s * 0.5f + 16.0f));
            sprB->setVisible(true);
            sprT->setVisible(true);
        } else {
            // Fallback: draw placeholder circle if texture not available
            float radius = s * (0.15f + 0.08f * std::max(0, cp.stage));
            Color4F col(0.95f, 0.85f, 0.35f, 1.0f);
            _cropsDraw->drawSolidCircle(center, radius, 0.0f, 12, col);
            _cropsDraw->drawCircle(center, radius, 0.0f, 12, false, Color4F(0,0,0,0.35f));
            if (cp.stage >= cp.maxStage) {
                _cropsDraw->drawCircle(center, radius + 2.0f, 0.0f, 12, false, Color4F(1.f,0.9f,0.2f,0.8f));
            }
        }
    }

    std::vector<long long> toRemove;
    for (auto& kv : _cropSprites) {
        if (alive.find(kv.first) == alive.end()) {
            toRemove.push_back(kv.first);
        }
    }
    for (auto k : toRemove) {
        if (_cropSprites.count(k)) {
            auto spr = _cropSprites[k];
            if (spr) spr->removeFromParent();
            _cropSprites.erase(k);
        }
        if (_cropSpritesTop.count(k)) {
            auto spr2 = _cropSpritesTop[k];
            if (spr2) spr2->removeFromParent();
            _cropSpritesTop.erase(k);
        }
    }
}

void FarmMapController::refreshDropsVisuals() {
    _dropSystem.refreshVisuals();
}

void FarmMapController::spawnDropAt(int c, int r, int itemType, int qty) {
    _dropSystem.spawnDropAt(this, c, r, itemType, qty);
}

void FarmMapController::collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv) {
    _dropSystem.collectDropsNear(playerWorldPos, inv);
}

void FarmMapController::setAllPlantableTilesWatered() {
    bool changed = false;
    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _cols; ++c) {
            int idx = r * _cols + c;
            if (idx < 0 || idx >= static_cast<int>(_tiles.size())) continue;
            if (_tiles[static_cast<std::size_t>(idx)] == Game::TileType::Tilled) {
                _tiles[static_cast<std::size_t>(idx)] = Game::TileType::Watered;
                changed = true;
            }
        }
    }
    if (!changed) return;
    Game::globalState().farmTiles = _tiles;
    refreshMapVisuals();
}

 

void FarmMapController::addActorToMap(cocos2d::Node* node, int /*zOrder*/) {
    if (_actorsRoot) {
        _actorsRoot->addChild(node, 0);
        return;
    }
    if (_farmMap && _farmMap->getTMX()) {
        _farmMap->getTMX()->addChild(node, 20);
    } else if (_worldNode) {
        _worldNode->addChild(node, 20);
    }
}
// namespace Controllers
}
