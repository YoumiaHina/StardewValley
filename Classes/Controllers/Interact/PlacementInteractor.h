#pragma once

#include "cocos2d.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/Map/RoomMapController.h"

namespace Controllers {

// PlacementInteractor：为“可放置物体”（箱子、熔炉等）选择放置中心点的工具类。
// - 职责：根据地图类型（农场/室内/室外）和玩家位置，计算一个合适的世界坐标 center。
// - 协作：由 ChestController/FurnaceController 等系统在 tryPlace 中调用。
// - 设计：所有函数均为 static，类本身不需要实例化，因此没有成员变量。
class PlacementInteractor {
public:
    // 农场放置中心：
    // - map       ：当前地图控制器，必须为农场地图（isFarm() 为 true）。
    // - playerPos ：玩家世界坐标。
    // - lastDir   ：玩家最近一次移动方向，用于选择“面前的格子”。
    // - outCenter ：输出参数，返回选中的世界坐标中心。
    // 实现：
    //   1. 调用 map->targetTile(playerPos, lastDir) 得到目标瓦片坐标 (tc,tr)。
    //   2. 检查是否在地图边界内。
    //   3. 使用 tileToWorld(tc,tr) 把瓦片坐标转换成世界坐标中心。
    static bool selectFarmCenter(IMapController* map,
                                 const cocos2d::Vec2& playerPos,
                                 const cocos2d::Vec2& lastDir,
                                 cocos2d::Vec2& outCenter);

    // 室内放置中心：
    // - 直接使用玩家当前位置 playerPos 作为放置中心（室内地图通常比较紧凑）。
    static bool selectRoomCenter(RoomMapController* room,
                                 const cocos2d::Vec2& playerPos,
                                 cocos2d::Vec2& outCenter);

    // 室外放置中心（Town/Beach 等）：
    // - 当前实现同样直接使用 playerPos，后续如有更复杂规则可在此扩展。
    static bool selectOutdoorCenter(IMapController* map,
                                    const cocos2d::Vec2& playerPos,
                                    cocos2d::Vec2& outCenter);
};

// inline 关键字：把函数定义放在头文件中，编译器在合适时可进行内联优化。
inline bool PlacementInteractor::selectFarmCenter(IMapController* map,
                                                     const cocos2d::Vec2& playerPos,
                                                     const cocos2d::Vec2& lastDir,
                                                     cocos2d::Vec2& outCenter) {
    if (!map) return false;
    if (!map->isFarm()) return false;
    auto tgt = map->targetTile(playerPos, lastDir);
    int tc = tgt.first;
    int tr = tgt.second;
    if (!map->inBounds(tc, tr)) return false;
    outCenter = map->tileToWorld(tc, tr);
    return true;
}

inline bool PlacementInteractor::selectRoomCenter(RoomMapController* room,
                                                     const cocos2d::Vec2& playerPos,
                                                     cocos2d::Vec2& outCenter) {
    if (!room) return false;
    outCenter = playerPos;
    return true;
}

inline bool PlacementInteractor::selectOutdoorCenter(IMapController* map,
                                                        const cocos2d::Vec2& playerPos,
                                                        cocos2d::Vec2& outCenter) {
    if (!map) return false;
    outCenter = playerPos;
    return true;
}

} // namespace Controllers
