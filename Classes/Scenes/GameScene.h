/**
 * GameScene: Main game scene placeholder.
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "ui/CocosGUI.h"
#include "Game/Inventory.h"
#include "Game/Tool.h"
#include "Game/GameConfig.h"
#include "Game/Tile.h"
#include "Game/Drop.h"
#include "Game/Chest.h"
#include "Game/Crop.h"

#include "Game/GameMap.h"
#include "Game/PlayerAppearance.h"

class GameScene : public cocos2d::Scene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(GameScene);

    void update(float dt) override;

    // 供其他场景调用：将玩家出生在农场门外侧
    void setSpawnAtFarmEntrance();

private:
    // 世界容器节点：承载地图、玩家、掉落物，便于相机平移
    cocos2d::Node* _worldNode = nullptr;
    Game::PlayerAppearance* _player = nullptr;
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

    Game::GameMap* _gameMap = nullptr;
    float _playerRadius = 12.0f;

    // drops & items
    std::vector<Game::Drop> _drops;
    cocos2d::Node* _dropsNode = nullptr;
    cocos2d::DrawNode* _dropsDraw = nullptr;

    // chests: placed storage
    std::vector<Game::Chest> _chests;
    cocos2d::DrawNode* _chestDraw = nullptr;
    bool _nearChest = false;
    cocos2d::Label* _chestPrompt = nullptr;

    std::vector<Game::Crop> _crops;
    cocos2d::DrawNode* _cropsDraw = nullptr;


    // crafting UI
    cocos2d::Node* _craftNode = nullptr;
    cocos2d::Label* _craftLabel = nullptr;
    cocos2d::ui::Button* _craftButton = nullptr;
    struct CraftRecipe { Game::ItemType result; Game::ItemType costType; int costQty; };
    std::vector<CraftRecipe> _recipes;

    // chest storage UI
    cocos2d::Node* _chestPanel = nullptr;
    cocos2d::Node* _chestListNode = nullptr;
    int _activeChestIdx = -1;
    struct WithdrawRow {
        Game::ItemType type;
        cocos2d::Label* nameLabel = nullptr;
        cocos2d::Label* countLabel = nullptr;
        cocos2d::Label* planLabel = nullptr;
        cocos2d::ui::Button* minusBtn = nullptr;
        cocos2d::ui::Button* plusBtn = nullptr;
        cocos2d::ui::Button* takeBtn = nullptr;
        int planQty = 0;
    };
    std::vector<WithdrawRow> _withdrawRows;

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

    // chests helpers
    void refreshChestsVisuals();
    void checkChestRegion();
    bool tileHasChest(int c, int r) const;

    void refreshCropsVisuals();
    int findCropIndex(int c, int r) const;
    void plantCrop(Game::CropType type, int c, int r);
    void advanceCropsDaily();
    void harvestCropAt(int c, int r);


    // crafting helpers
    void buildCraftUI();
    void refreshCraftUI();
    void craftChest();

    // chest UI helpers
    void buildChestUI();
    void refreshChestUI();
    void showChestPanel(int idx);
    void attemptWithdraw(Game::ItemType type, int qty);

    // chest panel geometry and drag & drop
    float _chestPanelW = 360.f;
    float _chestPanelH = 240.f;
    float _chestRowStartY = 60.f;
    float _chestRowGapY = 60.f;
    enum class DragSource { None, Inventory, Chest };
    DragSource _dragSource = DragSource::None;
    bool _dragging = false;
    int _dragSlotIndex = -1;
    Game::ItemType _dragType;
    int _dragQty = 0;
    cocos2d::Label* _dragGhost = nullptr;
};
