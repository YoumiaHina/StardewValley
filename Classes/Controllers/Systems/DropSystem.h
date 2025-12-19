/**
 * DropSystem: 掉落物系统（唯一来源）。
 * - 职责：维护地图掉落物列表（类型/数量/坐标）与拾取规则，并统一管理其渲染节点挂接与刷新。
 * - 协作对象：IMapController 提供 tileToWorld/inBounds 等坐标能力；Inventory 作为拾取目标；MapController 仅转发调用与提供挂载点解析。
 */
#pragma once

#include <functional>
#include <vector>
#include "cocos2d.h"
#include "Controllers/Map/IMapController.h"
#include "Game/Drop.h"

namespace Controllers {

class DropSystem {
public:
    struct AttachTarget {
        cocos2d::Node* parent = nullptr;
        int zOrder = 19;
    };

    // 配置：由 MapController 提供挂载点（TMX/worldNode）与层级。
    void configureTargetProvider(std::function<AttachTarget()> provider);

    // 设置掉落列表（常用于从存档/全局状态加载后一次性同步）。
    void setDrops(const std::vector<Game::Drop>& drops);

    // 读取当前掉落列表（只读）。
    const std::vector<Game::Drop>& drops() const;

    // 清空掉落并移除对应渲染节点。
    void clear();

    // 在指定 tile 上生成掉落（内部会校验 inBounds/qty，并自动刷新渲染与变更回调）。
    void spawnDropAt(Controllers::IMapController* map, int c, int r, int itemType, int qty);

    // 拾取玩家附近掉落（拾取后自动刷新渲染与变更回调）。
    void collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv);

    // 刷新渲染（当挂载点或掉落列表变化时调用）。
    void refreshVisuals();

    // 设置当掉落列表变化时的回调（用于持久化到 WorldState 等外部存储）。
    void setOnDropsChanged(std::function<void(const std::vector<Game::Drop>&)> cb);

private:
    void ensureAttached();
    void notifyChanged();

private:
    std::function<AttachTarget()> _targetProvider;
    std::function<void(const std::vector<Game::Drop>&)> _onDropsChanged;

    std::vector<Game::Drop> _drops;

    cocos2d::Node* _attachedParent = nullptr;
    int _attachedZOrder = 19;
    cocos2d::DrawNode* _dropsDraw = nullptr;
    cocos2d::Node* _dropsRoot = nullptr;
};

} // namespace Controllers

