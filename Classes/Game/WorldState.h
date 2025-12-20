/**
 * WorldState: 全局持久化的游戏状态（背包、农场地图与掉落等）。
 */
#pragma once

#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <string>
#include "Game/Inventory.h"
#include "Game/Tile.h"
#include "Game/Drop.h"
#include "Game/Chest.h"
#include "Game/GameConfig.h"
#include "Game/Crops/crop/CropBase.h"
#include "Game/Tree.h"
#include "Game/Rock.h"
#include "Game/Weed.h"
#include "Game/Animal.h"
#include "Game/Furnace.h"

namespace Game {

struct NpcQuest {
    std::string title;
    std::string description;
};

enum class SceneKind {
    Room = 0,
    Farm = 1,
    Mine = 2,
    Beach = 3,
    Town = 4
};

struct WorldState {
    // 共享背包实例（室内外一致）
    std::shared_ptr<Inventory> inventory;

    // 全局箱子（相当于扩展背包）
    Chest globalChest;

    // 农场地图（按行主序 r*_cols + c）
    std::vector<TileType> farmTiles;
    int farmCols = 0;
    int farmRows = 0;

    // 农场掉落（未拾取的物品）
    std::vector<Drop> farmDrops;

    // 农场熔炉
    std::vector<Furnace> farmFurnaces;

    // 室内熔炉
    std::vector<Furnace> houseFurnaces;

    // 城镇熔炉
    std::vector<Furnace> townFurnaces;

    // 沙滩熔炉
    std::vector<Furnace> beachFurnaces;

    // 农场箱子（已放置的储物箱）
    std::vector<Chest> farmChests;

    // 农场作物
    std::vector<Crop> farmCrops;

    std::vector<TreePos> farmTrees;
    std::vector<RockPos> farmRocks;
    std::vector<WeedPos> farmWeeds;

    std::vector<Animal> farmAnimals;


    // 室内箱子（房屋内放置的储物箱）
    std::vector<Chest> houseChests;

    // 城镇箱子
    std::vector<Chest> townChests;

    // 沙滩箱子
    std::vector<Chest> beachChests;

    // 热键选中槽位索引
    int selectedIndex = 0;

    // 玩家外观（捏脸）：用于存档持久化与跨场景还原
    int playerShirt = 0;
    int playerPants = 0;
    int playerHair = 0;
    int playerHairR = 255;
    int playerHairG = 255;
    int playerHairB = 255;

    // 时间系统：四季与天数（每季 30 天）
    int seasonIndex = 0;   // 0: Spring, 1: Summer, 2: Fall, 3: Winter
    int dayOfSeason = 1;   // 1..30
    int timeHour = 6;      // 0..23, default morning 06:00
    int timeMinute = 0;    // 0..59
    float timeAccum = 0.0f; // real seconds accumulator for minute advancement

    // 能量系统
    int energy = GameConfig::ENERGY_MAX;
    int maxEnergy = GameConfig::ENERGY_MAX;

    // 水壶水量系统
    int water = GameConfig::WATERING_CAN_MAX;
    int maxWater = GameConfig::WATERING_CAN_MAX;

    // 经济系统
    long long gold = 500;

    // 生命值系统（矿洞 HUD 专属显示）
    int hp = 1000;
    int maxHp = 1000;

    // 入口赠剑标记：仅在第一次进入矿洞0层时赠送一次
    bool grantedSwordAtEntrance = false;

    // 已激活电梯楼层（5 的倍数），长期保存，不因进出矿洞而清空
    std::unordered_set<int> abyssElevatorFloors;

    std::unordered_map<int, int> npcFriendship;
    std::unordered_map<int, bool> npcRomanceUnlocked;
    std::unordered_map<int, int> npcLastGiftDay;
    std::unordered_map<int, std::vector<NpcQuest>> npcQuests;
    bool fishingActive = false;
    bool isRaining = false;
    int weatherSeasonIndex = -1;
    int weatherDayOfSeason = -1;
    bool showCollisionDebug = false;

    // 技能树存档进度：
    // - 作为所有技能树经验/点数/解锁节点的唯一持久化来源。
    // - 与 SkillTreeSystem 协作：系统层负责把业务加经验与 UI 查询统一到这些字段。
    struct SkillTreeProgress {
        int totalXp = 0;
        int unspentPoints = 0;
        std::vector<int> unlockedNodeIds;
    };
    std::array<SkillTreeProgress, 6> skillTrees;

    int lastScene = static_cast<int>(SceneKind::Room);
    float lastPlayerX = 0.0f;
    float lastPlayerY = 0.0f;
    int lastMineFloor = 0;
    int lastSaveSlot = 1;

    bool pendingPassOut = false;
};

// 获取全局状态（惰性初始化由调用方保证）
WorldState& globalState();

} // namespace Game
