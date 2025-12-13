#include "Controllers/Map/AbyssMapController.h"
#include "cocos2d.h"
#include "Game/WorldState.h"
#include "Game/Tool/ToolFactory.h"

using namespace cocos2d;

namespace Controllers {

cocos2d::Size AbyssMapController::getContentSize() const {
    if (_entrance) return _entrance->getContentSize();
    if (_floorMap) return _floorMap->getContentSize();
    return Size(_cols * GameConfig::TILE_SIZE, _rows * GameConfig::TILE_SIZE);
}

cocos2d::Vec2 AbyssMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    Vec2 candidate = next;
    Size content = getContentSize();
    float minX = s * 0.5f;
    float minY = s * 0.5f;
    float maxX = content.width - s * 0.5f;
    float maxY = content.height - s * 0.5f;
    candidate.x = std::max(minX, std::min(maxX, candidate.x));
    candidate.y = std::max(minY, std::min(maxY, candidate.y));
    auto collidesFn = [this](const Vec2& p, float r){
        if (_entrance) return _entrance->collides(p, r);
        if (_floorMap) return _floorMap->collides(p, r);
        return false;
    };
    Vec2 foot(candidate.x, current.y);
    if (collidesFn(foot + Vec2(0, -s * 0.5f), radius)) candidate.x = current.x;
    foot = Vec2(current.x, candidate.y);
    if (collidesFn(foot + Vec2(0, -s * 0.5f), radius)) candidate.y = current.y;
    return candidate;
}

bool AbyssMapController::isNearDoor(const Vec2& playerWorldPos) const {
    // 用楼梯位置代替“门”提示
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    if (_entrance) return _entrance->nearStairs(playerWorldPos, s * 0.8f);
    if (_floorMap) return _floorMap->nearStairs(playerWorldPos, s * 0.8f);
    return playerWorldPos.distance(_stairsPos) <= s * 0.8f;
}

bool AbyssMapController::inBounds(int c, int r) const {
    return c >= 0 && r >= 0 && c < _cols && r < _rows;
}

std::pair<int,int> AbyssMapController::targetTile(const Vec2& playerPos, const Vec2& lastDir) const {
    int pc, pr; worldToTileIndex(playerPos, pc, pr);
    int dc = (lastDir.x > 0.1f) ? 1 : ((lastDir.x < -0.1f) ? -1 : 0);
    int dr = (lastDir.y > 0.1f) ? 1 : ((lastDir.y < -0.1f) ? -1 : 0);
    int tc = pc + dc; int tr = pr + dr;
    if (dc == 0 && dr == 0) { tr = pr - 1; }
    tc = std::max(0, std::min(_cols - 1, tc));
    tr = std::max(0, std::min(_rows - 1, tr));
    return { tc, tr };
}

void AbyssMapController::updateCursor(const Vec2& playerPos, const Vec2& lastDir) {
    if (!_mapDraw) return;
    // 简化：不绘制独立光标，地形已足够提示
}

Game::TileType AbyssMapController::getTile(int c, int r) const {
    return _tiles[r * _cols + c];
}

void AbyssMapController::setTile(int c, int r, Game::TileType t) {
    _tiles[r * _cols + c] = t;
}

Vec2 AbyssMapController::tileToWorld(int c, int r) const {
    if (_entrance) return _entrance->tileToWorld(c, r);
    if (_floorMap) return _floorMap->tileToWorld(c, r);
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
}

void AbyssMapController::worldToTileIndex(const Vec2& p, int& c, int& r) const {
    if (_entrance) { _entrance->worldToTileIndex(p, c, r); return; }
    if (_floorMap) { _floorMap->worldToTileIndex(p, c, r); return; }
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    c = static_cast<int>(p.x / s);
    r = static_cast<int>(p.y / s);
}

void AbyssMapController::refreshMapVisuals() {
    if (_entrance) {
        // Entrance TMX handles all visuals; 不绘制额外标记（去掉黄色圆圈）
        if (!_mapDraw && _entrance && _entrance->getTMX()) {
            _mapDraw = DrawNode::create();
            _entrance->getTMX()->addChild(_mapDraw, 50);
        }
        if (_mapDraw) _mapDraw->clear();
        _stairsPos = _entrance->stairsCenter();
        return;
    }
    if (_floorMap) {
        if (!_mapDraw && _floorMap && _floorMap->getTMX()) {
            _mapDraw = DrawNode::create();
            _floorMap->getTMX()->addChild(_mapDraw, 50);
        }
        if (_mapDraw) _mapDraw->clear();
        _stairsPos = _floorMap->stairsCenter();
        return;
    }
    if (!_mapDraw) {
        _mapDraw = DrawNode::create();
        if (_worldNode) _worldNode->addChild(_mapDraw, -1);
    }
    _mapDraw->clear();
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    Color4F base;
    switch (currentTheme()) {
        case Theme::Rock: base = Color4F(0.40f,0.35f,0.30f,1.0f); break;
        case Theme::Ice:  base = Color4F(0.30f,0.45f,0.65f,1.0f); break;
        case Theme::Lava: base = Color4F(0.55f,0.30f,0.20f,1.0f); break;
    }
    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _cols; ++c) {
            auto center = tileToWorld(c, r);
            Vec2 a(center.x - s/2, center.y - s/2);
            Vec2 b(center.x + s/2, center.y - s/2);
            Vec2 c2(center.x + s/2, center.y + s/2);
            Vec2 d(center.x - s/2, center.y + s/2);
            Vec2 rect[4] = { a,b,c2,d };
            _mapDraw->drawSolidPoly(rect, 4, base);
            _mapDraw->drawLine(a,b, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(b,c2,Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(c2,d, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(d,a, Color4F(0,0,0,0.25f));
        }
    }
    // 楼梯标记
    _mapDraw->drawSolidCircle(_stairsPos, s*0.4f, 0.0f, 16, Color4F(0.95f,0.85f,0.15f,1.0f));
}

void AbyssMapController::addActorToMap(cocos2d::Node* node, int zOrder) {
    if (_entrance && _worldNode) {
        // 入口层：人物置于世界节点最上层，避免被 TMX 图层遮挡且在卸载入口时不被移除
        _worldNode->addChild(node, 999);
    } else if (_floorMap && _floorMap->getTMX()) {
        _floorMap->getTMX()->addChild(node, 20);
    } else if (_worldNode) {
        _worldNode->addChild(node, zOrder);
    }
}

void AbyssMapController::generateFloor(int floorIndex) {
    _floor = std::max(1, std::min(120, floorIndex));
    if (_entrance) { _entrance->removeFromParent(); _entrance = nullptr; }
    if (_floorMap) { _floorMap->removeFromParent(); _floorMap = nullptr; }
    loadFloorTMX(_floor);
}

void AbyssMapController::descend(int by) {
    if (_entrance) { setFloor(1); return; }
    setFloor(_floor + by);
}

void AbyssMapController::setFloor(int floorIndex) {
    generateFloor(floorIndex);
    unlockElevatorIfNeeded();
}

AbyssMapController::Theme AbyssMapController::currentTheme() const {
    if (_floor <= 40) return Theme::Rock;
    if (_floor <= 80) return Theme::Ice;
    return Theme::Lava;
}

void AbyssMapController::unlockElevatorIfNeeded() {
    if (_floor % 5 == 0) {
        _elevatorFloors.insert(_floor);
        // 写回持久化集合
        auto &ws = Game::globalState();
        ws.abyssElevatorFloors.insert(_floor);
    }
}

bool AbyssMapController::isNearStairs(const Vec2& p) const {
    if (_entrance) {
        float s = static_cast<float>(GameConfig::TILE_SIZE);
        return _entrance->nearStairs(p, s * 0.8f);
    }
    if (_floorMap) {
        float s = static_cast<float>(GameConfig::TILE_SIZE);
        return _floorMap->nearStairs(p, s * 0.8f);
    }
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return p.distance(_stairsPos) <= s * 0.8f;
}

bool AbyssMapController::isNearFarmDoor(const Vec2& p) const {
    if (_entrance) {
        return _entrance->nearDoorToFarm(p);
    }
    return false;
}

bool AbyssMapController::isNearBack0(const Vec2& p) const {
    if (_floorMap) {
        return _floorMap->nearBack0(p);
    }
    return false;
}

bool AbyssMapController::isNearElestair(const Vec2& p) const {
    if (_entrance) {
        return _entrance->nearElestair(p);
    }
    return false;
}

std::vector<int> AbyssMapController::getActivatedElevatorFloors() const {
    std::vector<int> v;
    v.reserve(_elevatorFloors.size());
    for (int f : _elevatorFloors) v.push_back(f);
    std::sort(v.begin(), v.end());
    return v;
}

void AbyssMapController::loadEntrance() {
    if (!_worldNode) return;
    if (!_mapNode) { _mapNode = Node::create(); _worldNode->addChild(_mapNode, 0); }
    // 清理当前楼层 TMX，切换到入口
    if (_floorMap) { _floorMap->removeFromParent(); _floorMap = nullptr; }
    if (_entrance) { _entrance->removeFromParent(); _entrance = nullptr; }
    // 同步电梯楼层（持久化）到控制器
    {
        auto &ws = Game::globalState();
        if (!ws.abyssElevatorFloors.empty()) {
            _elevatorFloors.clear();
            for (int f : ws.abyssElevatorFloors) _elevatorFloors.insert(f);
        }
    }
    _entrance = Game::MineMap::create("Maps/mine/mine_0.tmx");
    if (_entrance) {
        _floor = 0; // 标记为零层入口
        _entrance->setAnchorPoint(Vec2(0,0));
        _entrance->setPosition(Vec2(0,0));
        _mapNode->addChild(_entrance, 0);
        auto tsize = _entrance->getMapSize();
        _cols = static_cast<int>(tsize.width);
        _rows = static_cast<int>(tsize.height);
        _stairsPos = _entrance->stairsCenter();
        refreshMapVisuals();

        // 首次进入矿洞0层：仅赠送一次剑（控制器内处理，避免场景包含业务逻辑）
        auto &ws = Game::globalState();
        if (!ws.grantedSwordAtEntrance && ws.inventory) {
            bool hasSword = false;
            for (std::size_t i = 0; i < ws.inventory->size(); ++i) {
                if (auto t = ws.inventory->toolAt(i)) {
                    if (t->kind() == Game::ToolKind::Sword) { hasSword = true; break; }
                }
            }
            bool inserted = false;
            if (!hasSword) {
                for (std::size_t i = 0; i < ws.inventory->size(); ++i) {
                    if (ws.inventory->isEmpty(i)) {
                        ws.inventory->setTool(i, Game::makeTool(Game::ToolKind::Sword));
                        inserted = true;
                        break;
                    }
                }
            }
            if (inserted || hasSword) {
                ws.grantedSwordAtEntrance = true;
            }
        }
    }
}

void AbyssMapController::loadFloorTMX(int floorIndex) {
    if (!_worldNode) return;
    if (!_mapNode) { _mapNode = Node::create(); _worldNode->addChild(_mapNode, 0); }
    std::string path;
    if (floorIndex % 5 == 0) {
        path = "Maps/mine/mine_bonusroom.tmx";
    } else {
        // 随机选择普通层地图
        std::mt19937 rng{ std::random_device{}() };
        std::uniform_int_distribution<int> dist(0,1);
        path = dist(rng) == 0 ? "Maps/mine/mine_corridor.tmx" : "Maps/mine/mine_room.tmx";
    }
    _floorMap = Game::MineMap::create(path);
    if (_floorMap) {
        _floorMap->setAnchorPoint(Vec2(0,0));
        _floorMap->setPosition(Vec2(0,0));
        _mapNode->addChild(_floorMap, 0);
        auto tsize = _floorMap->getMapSize();
        _cols = static_cast<int>(tsize.width);
        _rows = static_cast<int>(tsize.height);
        _stairsPos = _floorMap->stairsCenter();
        refreshMapVisuals();
    }
}

cocos2d::Vec2 AbyssMapController::entranceSpawnPos() const {
    if (_entrance) {
        auto appear = _entrance->appearCenter();
        if (appear != Vec2::ZERO) return appear;
        if (_stairsPos != Vec2::ZERO) return _stairsPos;
    }
    // fallback: center-top area
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return Vec2(_cols * s * 0.5f, _rows * s * 0.65f);
}

cocos2d::Vec2 AbyssMapController::entranceBackSpawnPos() const {
    if (_entrance) {
        auto back = _entrance->backAppearCenter();
        if (back != Vec2::ZERO) return back;
        // 若未配置 BackAppear，回退到 Appear 或楼梯
        auto appear = _entrance->appearCenter();
        if (appear != Vec2::ZERO) return appear;
        if (_stairsPos != Vec2::ZERO) return _stairsPos;
    }
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return Vec2(_cols * s * 0.5f, _rows * s * 0.65f);
}

cocos2d::Vec2 AbyssMapController::floorSpawnPos() const {
    if (_floorMap) {
        auto appear = _floorMap->appearCenter();
        if (appear != Vec2::ZERO) return appear;
        if (_stairsPos != Vec2::ZERO) return _stairsPos;
    }
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return Vec2(_cols * s * 0.5f, _rows * s * 0.65f);
}

const std::vector<cocos2d::Vec2>& AbyssMapController::monsterSpawnPoints() const {
    static std::vector<Vec2> empty;
    if (_floorMap) return _floorMap->monsterSpawnPoints();
    return empty;
}

const std::vector<cocos2d::Rect>& AbyssMapController::rockAreaRects() const {
    static std::vector<Rect> empty;
    if (_floorMap) return _floorMap->rockAreaRects();
    return empty;
}

const std::vector<std::vector<cocos2d::Vec2>>& AbyssMapController::rockAreaPolys() const {
    static std::vector<std::vector<Vec2>> empty;
    if (_floorMap) return _floorMap->rockAreaPolys();
    return empty;
}
bool AbyssMapController::collides(const Vec2& pos, float radius) const {
    if (_entrance) return _entrance->collides(pos, radius);
    if (_floorMap) return _floorMap->collides(pos, radius);
    return false;
}
// namespace Controllers
}
