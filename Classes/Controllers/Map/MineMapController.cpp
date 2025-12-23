#include "Controllers/Map/MineMapController.h"
#include "cocos2d.h"
#include "Game/WorldState.h"
#include "Game/Tool/ToolFactory.h"
#include "Game/Map/MapBase.h"

using namespace cocos2d;

namespace Controllers {

MineMapController::MineMapController(cocos2d::Node* worldNode)
: _worldNode(worldNode), _mineralSystem(this), _stairSystem(this) {}
// 构造函数使用成员初始化列表初始化字段：
// - _worldNode 保存传入的世界根节点指针；
// - _mineralSystem(this) / _stairSystem(this) 把当前控制器指针传给系统，
//   方便系统在内部调用地图接口（例如坐标转换、楼梯写回）。

EnvironmentObstacleSystemBase* MineMapController::obstacleSystem(ObstacleKind kind) {
    // 矿洞中仅有“矿物/石头”类环境障碍，统一由 MineralSystem 管理；
    // 其它 ObstacleKind 返回 nullptr，表示不支持。
    if (kind == ObstacleKind::Mineral || kind == ObstacleKind::Rock) return &_mineralSystem;
    return nullptr;
}

const EnvironmentObstacleSystemBase* MineMapController::obstacleSystem(ObstacleKind kind) const {
    if (kind == ObstacleKind::Mineral || kind == ObstacleKind::Rock) return &_mineralSystem;
    return nullptr;
}

void MineMapController::setLastClickWorldPos(const cocos2d::Vec2& p) {
    _lastClickWorldPos = p;
    _hasLastClick = true;
}

Vec2 MineMapController::getPlayerPosition(const Vec2& playerMapLocalPos) const {
    if (!_worldNode) return playerMapLocalPos;
    // ref 作为中间参照节点：
    // - 对入口层：人物挂在 worldNode 下，从 worldNode 自己开始转换；
    // - 对普通楼层：人物挂在具体 TMX 节点下，需要先从 TMX 局部坐标转到世界，
    //   再从世界坐标转为 worldNode 的本地坐标。
    Node* ref = nullptr;
    if (_entrance) {
        ref = _worldNode;
    } else if (_floorMap && _floorMap->getTMX()) {
        ref = _floorMap->getTMX();
    } else {
        ref = _worldNode;
    }
    Vec2 world = ref ? ref->convertToWorldSpace(playerMapLocalPos) : playerMapLocalPos;
    return _worldNode->convertToNodeSpace(world);
}

cocos2d::Size MineMapController::getContentSize() const {
    if (_entrance) return _entrance->getContentSize();
    if (_floorMap) return _floorMap->getContentSize();
    // 若当前既没有入口 TMX，也没有楼层 TMX，则用列数/行数和 TILE_SIZE 计算一个
    // “逻辑尺寸”，供相机/碰撞等模块使用。
    return Size(_cols * GameConfig::TILE_SIZE, _rows * GameConfig::TILE_SIZE);
}

cocos2d::Vec2 MineMapController::clampPosition(const Vec2& current, const Vec2& next, float radius) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    Vec2 candidate = next;
    Size content = getContentSize();
    float minX = s * 0.5f;
    float minY = s * 0.5f;
    float maxX = content.width - s * 0.5f;
    float maxY = content.height - s * 0.5f;
    candidate.x = std::max(minX, std::min(maxX, candidate.x));
    candidate.y = std::max(minY, std::min(maxY, candidate.y));

    // collidesBase：封装“当前位置是否与地图/矿物/临时碰撞体相交”的逻辑。
    // 使用 lambda（匿名函数）写法，等价于在 C 中写一个静态辅助函数。
    auto collidesBase = [this](const Vec2& p, float r) {
        bool base = false;
        if (_entrance) base = _entrance->collides(p, r);
        else if (_floorMap) base = _floorMap->collides(p, r);
        if (base) return true;
        if (!_mineralSystem.isEmpty() && _mineralSystem.collides(p, r * 0.75f, GameConfig::TILE_SIZE)) return true;
        for (const auto& rc : _dynamicColliders) {
            if (rc.containsPoint(p)) return true;
        }
        return false;
    };

    // 先尝试只在 X 方向移动（保持 Y 不变），实现“贴墙滑动”的效果。
    Vec2 tryX(candidate.x, current.y);
    Vec2 footX = tryX + Vec2(0, -s * 0.5f);
    if (collidesBase(footX, radius)) {
        tryX.x = current.x;
    }

    // 再尝试只在 Y 方向移动（保持 X 不变）。
    Vec2 tryY(current.x, candidate.y);
    Vec2 footY = tryY + Vec2(0, -s * 0.5f);
    if (collidesBase(footY, radius)) {
        tryY.y = current.y;
    }

    return Vec2(tryX.x, tryY.y);
}

bool MineMapController::isNearDoor(const Vec2& playerWorldPos) const {
    // 用楼梯位置代替“门”提示
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    if (_floor >= 120) return false;
    if (_entrance) return _entrance->nearStairs(playerWorldPos, s * 0.8f);
    if (_floorMap) return _floorMap->nearStairs(playerWorldPos, s * 0.8f);
    return playerWorldPos.distance(_stairsPos) <= s * 0.8f;
}

bool MineMapController::inBounds(int c, int r) const {
    return c >= 0 && r >= 0 && c < _cols && r < _rows;
}

std::pair<int,int> MineMapController::targetTile(const Vec2& playerPos, const Vec2& lastDir) const {
    // 使用 TileSelector::selectForwardTile 计算玩家面前的“目标瓦片”。
    // 该函数接受多个回调参数：
    // - worldToTile：把世界坐标转为瓦片索引；
    // - inBounds   ：检查瓦片是否还在地图范围内；
    // - tileToWorld：把瓦片索引转回世界坐标，用于绘制光标。
    // 这里的 [this](...) {...} 写法就是 lambda，类似把 this 指针和函数一起
    // 打包成回调，供工具系统在内部反复调用。
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

void MineMapController::updateCursor(const Vec2& playerPos, const Vec2& lastDir) {
    if (!_cursor) {
        _cursor = DrawNode::create();
        cocos2d::Node* parent = nullptr;
        int z = 21;
        if (_entrance && _entrance->getTMX()) {
            parent = _entrance->getTMX();
        } else if (_floorMap && _floorMap->getTMX()) {
            parent = _floorMap->getTMX();
        } else if (_mapNode) {
            parent = _mapNode;
            z = 1;
        } else if (_worldNode) {
            parent = _worldNode;
            z = 1;
        }
        if (parent) {
            parent->addChild(_cursor, z);
        }
    }
    // 使用 TileSelector::drawFanCursor 在 _cursor 上绘制扇形光标。
    // 这里再次传入 worldToTileIndex / inBounds / tileToWorld 回调，让光标
    // 的选中范围与 targetTile 使用的逻辑保持一致。
    TileSelector::drawFanCursor(
        _cursor,
        playerPos,
        lastDir,
        [this](const Vec2& p, int& c, int& r) { worldToTileIndex(p, c, r); },
        [this](int c, int r) { return inBounds(c, r); },
        [this](int c, int r) { return tileToWorld(c, r); },
        tileSize());
}

Game::TileType MineMapController::getTile(int c, int r) const {
    if (c < 0 || r < 0 || c >= _cols || r >= _rows) return Game::TileType::NotSoil;
    size_t idx = static_cast<size_t>(r) * static_cast<size_t>(_cols) + static_cast<size_t>(c);
    if (idx < _tiles.size()) return _tiles[idx];
    return Game::TileType::NotSoil;
}

void MineMapController::setTile(int c, int r, Game::TileType t) {
    size_t idx = static_cast<size_t>(r) * static_cast<size_t>(_cols) + static_cast<size_t>(c);
    if (idx < _tiles.size()) {
        _tiles[idx] = t;
    }
}

Vec2 MineMapController::tileToWorld(int c, int r) const {
    if (_entrance) return _entrance->tileToWorld(c, r);
    if (_floorMap) return _floorMap->tileToWorld(c, r);
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return Vec2(c * s + s * 0.5f, r * s + s * 0.5f);
}

void MineMapController::worldToTileIndex(const Vec2& p, int& c, int& r) const {
    if (_entrance) { _entrance->worldToTileIndex(p, c, r); return; }
    if (_floorMap) { _floorMap->worldToTileIndex(p, c, r); return; }
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    c = static_cast<int>(p.x / s);
    r = static_cast<int>(p.y / s);
}

void MineMapController::refreshMapVisuals() {
    if (_entrance) {
        // Entrance TMX handles all visuals; 不绘制额外标记（去掉黄色圆圈）
        if (!_mapDraw && _entrance && _entrance->getTMX()) {
            _mapDraw = DrawNode::create();
            _entrance->getTMX()->addChild(_mapDraw, 50);
        }
        if (_mapDraw) _mapDraw->clear();
        // stairsCenter 从 TMX 对象层读取楼梯区域的中心点，用于交互判定。
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
        // 把 StairSystem 生成的额外楼梯位置写回到 MineMap（TMX 封装），
        // 然后让楼梯系统刷新可视化节点。
        _stairSystem.syncExtraStairsToMap(_minerals);
        _stairSystem.refreshVisuals();
        return;
    }
    if (!_mapDraw) {
        _mapDraw = DrawNode::create();
        if (_worldNode) _worldNode->addChild(_mapDraw, -1);
    }
    _mapDraw->clear();
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    // 根据当前楼层选择主题底色，Color4F(r,g,b,a) 各分量范围为 [0,1]。
    Color4F base;
    switch (currentTheme()) {
        case Theme::Rock: base = Color4F(0.40f,0.35f,0.30f,1.0f); break;
        case Theme::Ice:  base = Color4F(0.30f,0.45f,0.65f,1.0f); break;
        case Theme::Lava: base = Color4F(0.55f,0.30f,0.20f,1.0f); break;
    }
    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _cols; ++c) {
            auto center = tileToWorld(c, r);
            // 计算四个顶点，围成一个矩形瓦片。
            Vec2 a(center.x - s/2, center.y - s/2);
            Vec2 b(center.x + s/2, center.y - s/2);
            Vec2 c2(center.x + s/2, center.y + s/2);
            Vec2 d(center.x - s/2, center.y + s/2);
            Vec2 rect[4] = { a,b,c2,d };
            _mapDraw->drawSolidPoly(rect, 4, base);
            // 在边缘画浅色线条，形成简单的网格感。
            _mapDraw->drawLine(a,b, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(b,c2,Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(c2,d, Color4F(0,0,0,0.25f));
            _mapDraw->drawLine(d,a, Color4F(0,0,0,0.25f));
        }
    }
    // 用一个实心圆高亮主楼梯位置，便于调试和观察。
    _mapDraw->drawSolidCircle(_stairsPos, s*0.4f, 0.0f, 16, Color4F(0.95f,0.85f,0.15f,1.0f));
}

void MineMapController::refreshDropsVisuals() {
    _dropSystem.refreshVisuals();
}

void MineMapController::spawnDropAt(int c, int r, int itemType, int qty) {
    _dropSystem.spawnDropAt(this, c, r, itemType, qty);
}

void MineMapController::collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv) {
    _dropSystem.collectDropsNear(playerWorldPos, inv);
}

void MineMapController::addActorToMap(cocos2d::Node* node, int zOrder) {
    if (_entrance && _worldNode) {
        // 入口层：人物置于世界节点最上层，避免被 TMX 图层遮挡且在卸载入口时不被移除
        _worldNode->addChild(node, 999);
    } else if (_floorMap && _floorMap->getTMX()) {
        _floorMap->getTMX()->addChild(node, 20);
    } else if (_worldNode) {
        _worldNode->addChild(node, zOrder);
    }
}

void MineMapController::generateFloor(int floorIndex) {
    // 将 floorIndex 夹紧在 [1,120] 范围内，避免跳转到非法楼层。
    _floor = std::max(1, std::min(120, floorIndex));
    // 清理与上一层相关的运行时状态与节点：
    // - 光标节点 _cursor；
    // - 掉落物系统、额外楼梯、楼梯系统内部状态；
    // - 矿物数据与可视化；
    // - 入口/楼层 TMX 节点。
    if (_cursor) {
        _cursor->removeFromParent();
        _cursor = nullptr;
    }
    _dropSystem.clear();
    _extraStairs.clear();
    _stairSystem.reset();
    _minerals.clear();
    _mineralSystem.clearVisuals();
    _stairSystem.attachTo(nullptr);
    if (_entrance) { _entrance->removeFromParent(); _entrance = nullptr; }
    if (_floorMap) { _floorMap->removeFromParent(); _floorMap = nullptr; }
    loadFloorTMX(_floor);
}

void MineMapController::descend(int by) {
    if (_entrance) { setFloor(1); return; }
    setFloor(_floor + by);
}

void MineMapController::setFloor(int floorIndex) {
    generateFloor(floorIndex);
    unlockElevatorIfNeeded();
}

MineMapController::Theme MineMapController::currentTheme() const {
    if (_floor <= 40) return Theme::Rock;
    if (_floor <= 80) return Theme::Ice;
    return Theme::Lava;
}

void MineMapController::unlockElevatorIfNeeded() {
    if (_floor % 5 == 0) {
        _elevatorFloors.insert(_floor);
        // 写回持久化集合：使用 WorldState::abyssElevatorFloors 记录全局解锁进度，
        // 下次进入入口层时会同步回 _elevatorFloors。
        auto &ws = Game::globalState();
        ws.abyssElevatorFloors.insert(_floor);
    }
}

bool MineMapController::isNearStairs(const Vec2& p) const {
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float radius = s * 1.2f;
    if (_floor >= 120) {
        return false;
    }
    if (_entrance) {
        return _entrance->nearStairs(p, radius);
    }
    if (_floorMap) {
        bool mainCovered = false;
        if (_stairsPos != Vec2::ZERO && !_minerals.empty()) {
            int sc = 0;
            int sr = 0;
            worldToTileIndex(_stairsPos, sc, sr);
            for (const auto& m : _minerals) {
                int mc = 0;
                int mr = 0;
                worldToTileIndex(m.pos, mc, mr);
                if (mc == sc && mr == sr) {
                    mainCovered = true;
                    break;
                }
            }
        }
        if (!mainCovered) {
            if (_floorMap->nearStairs(p, radius)) {
                return true;
            }
            if (_floor > 0 && (_floor % 5 == 0) && _floorMap->nearElestair(p)) {
                return true;
            }
        }
        for (const auto& stairPos : _extraStairs) {
            if (p.distance(stairPos) <= radius) {
                return true;
            }
        }
        return false;
    }
    if (!_extraStairs.empty()) {
        for (const auto& stairPos : _extraStairs) {
            if (p.distance(stairPos) <= radius) {
                return true;
            }
        }
    }
    return p.distance(_stairsPos) <= radius;
}

bool MineMapController::isNearFarmDoor(const Vec2& p) const {
    if (_entrance) {
        return _entrance->nearDoorToFarm(p);
    }
    return false;
}

bool MineMapController::isNearBack0(const Vec2& p) const {
    if (_floorMap) {
        return _floorMap->nearBack0(p);
    }
    return false;
}

bool MineMapController::isNearElestair(const Vec2& p) const {
    if (_entrance) {
        return _entrance->nearElestair(p);
    }
    return false;
}

std::vector<int> MineMapController::getActivatedElevatorFloors() const {
    std::vector<int> v;
    v.reserve(_elevatorFloors.size());
    // 从 unordered_set 拷贝到 vector，并排序后供 UI 控件展示。
    for (int f : _elevatorFloors) v.push_back(f);
    std::sort(v.begin(), v.end());
    return v;
}

void MineMapController::loadEntrance() {
    if (!_worldNode) return;
    if (!_mapNode) { _mapNode = Node::create(); _worldNode->addChild(_mapNode, 0); }
    // 与 generateFloor 类似，先清理上一层相关状态，然后再创建新的入口 TMX。
    if (_cursor) {
        _cursor->removeFromParent();
        _cursor = nullptr;
    }
    _dropSystem.clear();
    _extraStairs.clear();
    _stairSystem.reset();
    _minerals.clear();
    _mineralSystem.clearVisuals();
    _stairSystem.attachTo(nullptr);
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
    // 通过工厂方法创建入口层封装对象，内部会加载 mine_0.tmx 等资源。
    _entrance = Game::MineMap::createEntrance();
    if (_entrance) {
        _floor = 0;
        // 设置锚点为 (0,0) 并放在 (0,0)，表示“左下角对齐世界原点”。
        _entrance->setAnchorPoint(Vec2(0,0));
        _entrance->setPosition(Vec2(0,0));
        _mapNode->addChild(_entrance, 0);
        // getMapSize 返回 TMX 的列数/行数（瓦片数量），用于初始化 _cols/_rows。
        auto tsize = _entrance->getMapSize();
        _cols = static_cast<int>(tsize.width);
        _rows = static_cast<int>(tsize.height);
        // 用 NotSoil 填满 _tiles，矿洞地图本身不依赖该数组做逻辑，仅用于调试绘制。
        _tiles.assign(static_cast<std::size_t>(_cols) * static_cast<std::size_t>(_rows), Game::TileType::NotSoil);
        _stairsPos = _entrance->stairsCenter();
        // MineralSystem / StairSystem 挂到入口 TMX 节点下，负责创建并管理相应精灵。
        _mineralSystem.attachTo(_entrance->getTMX());
        _mineralSystem.bindRuntimeStorage(&_minerals);
        _stairSystem.attachTo(_entrance->getTMX());
        // DropSystem 通过一个回调函数获取“应该把掉落物挂到哪里”，这里优先挂
        // 在 TMX 节点，其次是 worldNode。
        _dropSystem.configureTargetProvider([this]() -> Controllers::DropSystem::AttachTarget {
            Controllers::DropSystem::AttachTarget tgt;
            if (_entrance && _entrance->getTMX()) {
                tgt.parent = _entrance->getTMX();
                tgt.zOrder = 19;
            } else if (_floorMap && _floorMap->getTMX()) {
                tgt.parent = _floorMap->getTMX();
                tgt.zOrder = 19;
            } else if (_worldNode) {
                tgt.parent = _worldNode;
                tgt.zOrder = 19;
            }
            return tgt;
        });
        refreshMapVisuals();

        // 首次进入矿洞0层：仅赠送一次剑（控制器内处理，避免场景包含业务逻辑）
        auto &ws = Game::globalState();
        if (!ws.grantedSwordAtEntrance && ws.inventory) {
            bool hasSword = false; // 标记玩家当前是否已经持有剑
            for (std::size_t i = 0; i < ws.inventory->size(); ++i) {
                if (auto t = ws.inventory->toolAt(i)) {
                    if (t->kind() == Game::ToolKind::Sword) { hasSword = true; break; }
                }
            }
            bool inserted = false; // 标记是否在本次函数调用中成功发放了一把剑
            if (!hasSword) {
                // 遍历背包格子，寻找第一个空位；size() 返回格子数量。
                for (std::size_t i = 0; i < ws.inventory->size(); ++i) {
                    if (ws.inventory->isEmpty(i)) {
                        ws.inventory->setTool(i, Game::makeTool(Game::ToolKind::Sword));
                        inserted = true;
                        break;
                    }
                }
            }
            // 若玩家原本就有剑，或本次顺利发放了一把剑，则标记为已发放过，
            // 避免后续再次进入入口层时重复赠送。
            if (inserted || hasSword) {
                ws.grantedSwordAtEntrance = true;
            }
        }
    }
}

void MineMapController::loadFloorTMX(int floorIndex) {
    if (!_worldNode) return;
    if (!_mapNode) { _mapNode = Node::create(); _worldNode->addChild(_mapNode, 0); }
    // 通过工厂方法根据 floorIndex 创建对应楼层的 MineMap 封装对象。
    _floorMap = Game::MineMap::createFloor(floorIndex);
    if (_floorMap) {
        _floorMap->setAnchorPoint(Vec2(0,0));
        _floorMap->setPosition(Vec2(0,0));
        _mapNode->addChild(_floorMap, 0);
        auto tsize = _floorMap->getMapSize();
        _cols = static_cast<int>(tsize.width);
        _rows = static_cast<int>(tsize.height);
        _tiles.assign(static_cast<std::size_t>(_cols) * static_cast<std::size_t>(_rows), Game::TileType::NotSoil);
        _extraStairs.clear();
        _stairsPos = _floorMap->stairsCenter();
        _minerals.clear();
        _mineralSystem.bindRuntimeStorage(&_minerals);
        if (_floorMap->getTMX()) {
            _mineralSystem.attachTo(_floorMap->getTMX());
        } else {
            // 极端情况下若没有 TMX 节点，则挂在 mapNode 下。
            _mineralSystem.attachTo(_mapNode);
        }
        cocos2d::Node* stairRoot = nullptr;
        if (_floorMap->getTMX()) {
            stairRoot = _floorMap->getTMX();
        } else {
            stairRoot = _mapNode;
        }
        if (stairRoot) {
            _stairSystem.attachTo(stairRoot);
        }
        bool isBottom = (floorIndex >= 120); // 最底层不再生成额外楼梯
        std::vector<Vec2> candidates;        // 可放置矿石/楼梯的候选点
        candidates.reserve(static_cast<std::size_t>(_cols * _rows));
        float s = static_cast<float>(GameConfig::TILE_SIZE);
        const auto& rockRects = rockAreaRects();
        const auto& rockPolys = rockAreaPolys();
        for (int r = 0; r < _rows; ++r) {
            for (int c = 0; c < _cols; ++c) {
                Vec2 center = tileToWorld(c, r);
                // 使用 MapBase::collidesAt 判断以 center 为圆心、半径 s*0.5 的圆
                // 是否落在任意矿区区域中；若是，则加入候选列表。
                if (Game::MapBase::collidesAt(center, s * 0.5f, rockRects, rockPolys)) {
                    candidates.push_back(center);
                }
            }
        }
        if (!isBottom && !candidates.empty()) {
            // 在候选点中随机生成 2~4 个额外楼梯，写入 _extraStairs。
            _stairSystem.generateStairs(candidates, 2, 4, _extraStairs);
        }
        std::vector<Vec2> stairWorldPos;
        stairWorldPos.push_back(_stairsPos);
        for (const auto& sp : _extraStairs) {
            stairWorldPos.push_back(sp);
        }
        // 根据候选区域和楼梯位置生成矿石节点，避免矿石直接挡住楼梯。
        _mineralSystem.generateNodesForFloor(_minerals, candidates, stairWorldPos);
        _mineralSystem.syncVisuals();
        _stairSystem.syncExtraStairsToMap(_minerals);
        _stairSystem.refreshVisuals();
        refreshMapVisuals();
    }
}

cocos2d::Vec2 MineMapController::entranceSpawnPos() const {
    if (_entrance) {
        auto appear = _entrance->appearCenter();
        if (appear != Vec2::ZERO) return appear;
        if (_stairsPos != Vec2::ZERO) return _stairsPos;
    }
    // fallback: center-top area
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return Vec2(_cols * s * 0.5f, _rows * s * 0.65f);
}

cocos2d::Vec2 MineMapController::entranceBackSpawnPos() const {
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

cocos2d::Vec2 MineMapController::floorSpawnPos() const {
    if (_floorMap) {
        auto appear = _floorMap->appearCenter();
        if (appear != Vec2::ZERO) return appear;
        if (_stairsPos != Vec2::ZERO) return _stairsPos;
    }
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    return Vec2(_cols * s * 0.5f, _rows * s * 0.65f);
}

const std::vector<cocos2d::Vec2>& MineMapController::monsterSpawnPoints() const {
    static std::vector<Vec2> empty;
    if (_floorMap) return _floorMap->monsterSpawnPoints();
    return empty;
}

const std::vector<cocos2d::Rect>& MineMapController::rockAreaRects() const {
    static std::vector<Rect> empty;
    if (_floorMap) return _floorMap->rockAreaRects();
    return empty;
}

const std::vector<std::vector<cocos2d::Vec2>>& MineMapController::rockAreaPolys() const {
    static std::vector<std::vector<Vec2>> empty;
    if (_floorMap) return _floorMap->rockAreaPolys();
    return empty;
}
bool MineMapController::collides(const Vec2& pos, float radius) const {
    bool base = false;
    if (_entrance) base = _entrance->collides(pos, radius);
    else if (_floorMap) base = _floorMap->collides(pos, radius);
    if (base) return true;
    if (!_mineralSystem.isEmpty() && _mineralSystem.collides(pos, radius * 0.75f, GameConfig::TILE_SIZE)) return true;
    for (const auto& rc : _dynamicColliders) { if (rc.containsPoint(pos)) return true; }
    for (const auto& rc : _monsterColliders) { if (rc.containsPoint(pos)) return true; }
    return false;
}

bool MineMapController::collidesWithoutMonsters(const Vec2& pos, float radius) const {
    bool base = false;
    if (_entrance) base = _entrance->collides(pos, radius);
    else if (_floorMap) base = _floorMap->collides(pos, radius);
    if (base) return true;
    if (!_mineralSystem.isEmpty() && _mineralSystem.collides(pos, radius * 0.75f, GameConfig::TILE_SIZE)) return true;
    for (const auto& rc : _dynamicColliders) { if (rc.containsPoint(pos)) return true; }
    return false;
}
// namespace Controllers
}
