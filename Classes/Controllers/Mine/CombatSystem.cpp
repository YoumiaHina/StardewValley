#include "Controllers/Mine/CombatSystem.h"
#include "cocos2d.h"
#include "Game/WorldState.h"
#include "Game/Tool/ToolBase.h"
#include "Game/Tool/Weapon.h"
#include "Game/Tool/Sword.h"

using namespace cocos2d;

namespace Controllers {

// onMouseDown：
// 处理矿洞内的鼠标左键攻击逻辑。
// 流程：
// 1) 仅在左键按下时响应，其它按键直接忽略；
// 2) 通过回调获取玩家当前位置与当前选中的工具；
// 3) 若选中的是剑：
//    - 通过 Weapon 接口读取基础伤害与攻击冷却时间；
//    - 由 Game::Sword 提供攻击形状（前方扇形瓦片区域）；
//    - 使用 MineMapController 构建命中的瓦片列表；
//    - 将瓦片与基础伤害交给 MineMonsterController::applyAreaDamage，由其完成
//      怪物的具体伤害与死亡结算；
// 4) 若仍在攻击冷却时间内，则本次点击不触发伤害结算。
void MineCombatController::onMouseDown(EventMouse* e) {
    // cocos2d::EventMouse 提供 getMouseButton() 查询是哪一个鼠标键被按下，
    // 这里仅处理左键（BUTTON_LEFT），像 C 里用常量做分支判断。
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    // 正在冷却中：本次点击不触发攻击。
    if (_attackCooldownRemaining > 0.0f) return;
    // 通过回调获取玩家当前世界坐标；若回调为空则退化为 (0,0)
    Vec2 pos = _getPlayerPos ? _getPlayerPos() : Vec2();
    // Game::globalState() 返回一个全局单例，类似 C 里的全局结构体实例，
    // 这里从中拿到玩家背包指针，用于查询当前选中的工具。
    auto inv = Game::globalState().inventory;
    Game::ToolBase* selected = inv ? inv->selectedTool() : nullptr;
    // 仅当当前工具种类为 Sword 时才进行近战攻击判定。
    if (!selected || selected->kind() != Game::ToolKind::Sword || !_monsters || !_map) {
        return;
    }
    // Sword 继承自 Weapon，接口提供基础伤害与攻击冷却时间。
    auto* sword = static_cast<Game::Sword*>(selected);
    int baseDamage = sword->baseDamage();
    if (baseDamage <= 0) return;
    float cd = sword->attackCooldownSeconds();
    if (cd > 0.0f) {
        _attackCooldownRemaining = cd;
    }
    // 通过回调获取玩家最近一次移动方向，用作攻击扇形朝向；
    // 若回调不存在则默认朝上 (0,-1)。
    Vec2 dir = _getLastDir ? _getLastDir() : Vec2(0, -1);
    // 使用 std::vector 存储要命中的瓦片列表，元素是 (列, 行) 的整数对，
    // 等价于 C 里的“可自动扩容的二维整型数组”。
    std::vector<std::pair<int,int>> tiles;
    // buildHitTiles 会根据当前地图、玩家位置和朝向，将扇形范围内的瓦片索引
    // 写入 tiles 中；true 表示允许命中矿石等障碍。
    Game::Sword::buildHitTiles(_map, pos, dir, tiles, true);
    if (!tiles.empty()) {
        // 仅在至少命中一个瓦片时才调用怪物系统结算伤害。
        _monsters->applyAreaDamage(tiles, baseDamage);
    }
    return;
}

void MineCombatController::update(float dt) {
    if (_attackCooldownRemaining > 0.0f) {
        _attackCooldownRemaining -= dt;
        if (_attackCooldownRemaining < 0.0f) {
            _attackCooldownRemaining = 0.0f;
        }
    }
}

} // namespace Controllers
