#pragma once

#include "cocos2d.h"
#include <functional>
#include <utility>
#include <vector>

namespace Controllers {

// 瓦片选择工具：
// - 作用：根据玩家位置/朝向/最近一次点击等信息，计算工具作用的目标瓦片，并可绘制扇形光标。
// - 职责边界：只提供纯计算与绘制辅助，不持有地图状态与输入状态。
// - 主要协作对象：调用方通过回调提供 world<->tile 坐标转换与边界判断，避免耦合具体地图实现。
class TileSelector {
public:
    // 选择“前方目标瓦片”：优先使用最近一次点击，否则按朝向取面前瓦片。
    static std::pair<int,int> selectForwardTile(
        const cocos2d::Vec2& playerPos,
        const cocos2d::Vec2& lastDir,
        const std::function<void(const cocos2d::Vec2&, int&, int&)>& worldToTileIndex,
        const std::function<bool(int,int)>& inBounds,
        float tileSize,
        bool hasLastClick,
        const cocos2d::Vec2& lastClickWorldPos,
        const std::function<cocos2d::Vec2(int,int)>& tileToWorld);

    // 绘制扇形光标：根据朝向在前方扇形区域绘制可作用瓦片的提示。
    static void drawFanCursor(
        cocos2d::DrawNode* cursor,
        const cocos2d::Vec2& playerPos,
        const cocos2d::Vec2& lastDir,
        const std::function<void(const cocos2d::Vec2&, int&, int&)>& worldToTileIndex,
        const std::function<bool(int,int)>& inBounds,
        const std::function<cocos2d::Vec2(int,int)>& tileToWorld,
        float tileSize);

    // 收集前方扇形内的瓦片坐标列表（可选择是否包含玩家自身所在瓦片）。
    static void collectForwardFanTiles(
        const cocos2d::Vec2& playerPos,
        const cocos2d::Vec2& lastDir,
        const std::function<void(const cocos2d::Vec2&, int&, int&)>& worldToTileIndex,
        const std::function<bool(int,int)>& inBounds,
        float tileSize,
        bool includeSelf,
        std::vector<std::pair<int,int>>& outTiles);
};

}
