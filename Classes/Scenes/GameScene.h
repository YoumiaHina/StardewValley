/**
 * GameScene: Main game scene placeholder.
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "Game/Inventory.h"
#include "Game/Tool.h"
#include "Game/GameConfig.h"
#include "Game/Tile.h"
#include "Game/Drop.h"

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(GameScene);

    void update(float dt) override;

    // 供其他场景调用：将玩家出生在农场门外侧
    void setSpawnAtFarmEntrance();

private:
    cocos2d::DrawNode* _player = nullptr;
    // hotbar/inventory UI（共享）
    std::shared_ptr<Game::Inventory> _inventory;
    cocos2d::Node* _hotbarNode = nullptr;
    cocos2d::DrawNode* _hotbarHighlight = nullptr;
    std::vector<cocos2d::Label*> _hotbarLabels;
    // input state
    bool _up = false;
    bool _down = false;
    bool _left = false;
    bool _right = false;
    // movement params
    float _baseSpeed = 140.0f;      // pixels per second
    float _sprintSpeed = 240.0f;    // pixels per second when sprinting
    float _moveHeldDuration = 0.0f;
    bool  _isSprinting = false;
    const float _sprintThreshold = 0.5f; // seconds to hold W to sprint

    // helpers
    void buildHotbarUI();
    void refreshHotbarUI();
    void useSelectedTool();
    void buildHUD();
    void refreshHUD();

    // tile map
    int _cols = GameConfig::MAP_COLS;
    int _rows = GameConfig::MAP_ROWS;
    std::vector<Game::TileType> _tiles; // row-major: index = r * _cols + c
    cocos2d::Node* _mapNode = nullptr;
    cocos2d::DrawNode* _mapDraw = nullptr;
    cocos2d::DrawNode* _cursor = nullptr;
    cocos2d::Vec2 _mapOrigin; // bottom-left of grid in world coords
    cocos2d::Vec2 _lastDir = cocos2d::Vec2(0, -1); // default facing down

    // drops & items
    std::vector<Game::Drop> _drops;
    cocos2d::Node* _dropsNode = nullptr;
    cocos2d::DrawNode* _dropsDraw = nullptr;

    // HUD：时间与能量
    cocos2d::Label* _hudTimeLabel = nullptr;
    cocos2d::Node* _energyNode = nullptr;
    cocos2d::DrawNode* _energyFill = nullptr;
    cocos2d::Label* _energyLabel = nullptr;

    // 农场↔房间：门口区域与提示
    cocos2d::Rect _farmDoorRect;          // 农场房屋门口区域（靠近底部中间）
    cocos2d::Label* _doorPrompt = nullptr; // 靠近门口时的交互提示
    bool _nearFarmDoor = false;            // 玩家是否在门口附近

    void buildMap();
    void refreshMapVisuals();
    void updateCursor();
    bool inBounds(int c, int r) const;
    Game::TileType getTile(int c, int r) const;
    void setTile(int c, int r, Game::TileType t);
    cocos2d::Vec2 tileToWorld(int c, int r) const; // center position
    void worldToTileIndex(const cocos2d::Vec2& p, int& c, int& r) const;
    std::pair<int,int> targetTile() const;

    // drops helpers
    void refreshDropsVisuals();
    void spawnDropAt(int c, int r, Game::ItemType type, int qty = 1);
    void collectDropsNearPlayer();

    // 农场门口检测与切换（内部使用）
    void checkFarmDoorRegion();
};