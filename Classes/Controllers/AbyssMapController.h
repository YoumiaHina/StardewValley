/**
 * AbyssMapController: 深渊矿洞地图控制器（120层，三阶段主题）。
 * 管理楼层、主题、地形绘制、楼梯/电梯、障碍与可视。
 */
#pragma once

#include "cocos2d.h"
#include <vector>
#include <unordered_set>
#include "Controllers/IMapController.h"
#include "Game/GameConfig.h"

namespace Controllers {

class AbyssMapController : public IMapController {
public:
    enum class Theme { Rock, Ice, Lava };

    AbyssMapController(cocos2d::Node* worldNode)
    : _worldNode(worldNode) {}

    // IMapController overrides
    cocos2d::Size getContentSize() const override;
    cocos2d::Vec2 getOrigin() const override { return cocos2d::Vec2(0,0); }
    cocos2d::Vec2 clampPosition(const cocos2d::Vec2& current,
                                 const cocos2d::Vec2& next,
                                 float radius) const override;
    bool collides(const cocos2d::Vec2& pos, float radius) const override { return false; }
    bool isNearDoor(const cocos2d::Vec2& playerWorldPos) const override; // 门提示替换为“楼梯/电梯”，这里近似为楼梯区域
    bool isNearChest(const cocos2d::Vec2& playerWorldPos) const override { return false; }
    float tileSize() const override { return static_cast<float>(GameConfig::TILE_SIZE); }
    bool inBounds(int c, int r) const override;
    std::pair<int,int> targetTile(const cocos2d::Vec2& playerPos,
                                  const cocos2d::Vec2& lastDir) const override;
    void updateCursor(const cocos2d::Vec2& playerPos,
                      const cocos2d::Vec2& lastDir) override;
    Game::TileType getTile(int c, int r) const override;
    void setTile(int c, int r, Game::TileType t) override;
    cocos2d::Vec2 tileToWorld(int c, int r) const override;
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const override;
    void refreshMapVisuals() override;
    void refreshCropsVisuals() override {} // not used
    void refreshDropsVisuals() override {} // not used
    void spawnDropAt(int, int, int, int) override {} // not used here
    const std::vector<Game::Chest>& chests() const override { return _emptyChests; }
    std::vector<Game::Chest>& chests() override { return _emptyChests; }
    void addActorToMap(cocos2d::Node* node, int zOrder) override;

    // Abyss specific
    void generateFloor(int floorIndex);
    void descend(int by = 1);
    void setFloor(int floorIndex);
    int currentFloor() const { return _floor; }
    Theme currentTheme() const;
    void unlockElevatorIfNeeded();
    bool isNearStairs(const cocos2d::Vec2& p) const;
    cocos2d::Vec2 stairsWorldPos() const { return _stairsPos; }
    std::vector<int> getActivatedElevatorFloors() const;

private:
    cocos2d::Node* _worldNode = nullptr;
    cocos2d::DrawNode* _mapDraw = nullptr;
    int _cols = 80;
    int _rows = 60;
    std::vector<Game::TileType> _tiles; // 简化：使用 TileType 渲染主题色块
    int _floor = 1;
    cocos2d::Vec2 _stairsPos;
    std::unordered_set<int> _elevatorFloors; // 已激活楼层（5的倍数）
    std::vector<Game::Chest> _emptyChests; // 深渊内无胸，返回空引用
};

} // namespace Controllers
