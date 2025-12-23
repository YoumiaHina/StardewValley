#include "Controllers/Mine/CombatSystem.h"
#include "cocos2d.h"
#include "Game/WorldState.h"
#include "Game/Tool/ToolBase.h"
#include "Game/Tool/Sword.h"

using namespace cocos2d;

namespace Controllers {

// onMouseDown：
// 处理矿洞内的鼠标左键攻击逻辑。
// 流程：
// 1) 仅在左键按下时响应，其它按键直接忽略；
// 2) 通过回调获取玩家当前位置与当前选中的工具；
// 3) 若选中的是剑：
//    - 由 Game::Sword 提供基础伤害与攻击形状（前方扇形瓦片区域）；
//    - 使用 MineMapController 构建命中的瓦片列表；
//    - 将瓦片与基础伤害交给 MineMonsterController::applyAreaDamage，由其完成
//      怪物的具体伤害与死亡结算；
// 4) 当前实现中，若没选中剑则不执行任何攻击逻辑，为后续扩展预留空间。
void MineCombatController::onMouseDown(EventMouse* e) {
    // cocos2d::EventMouse 提供 getMouseButton() 查询是哪一个鼠标键被按下，
    // 这里仅处理左键（BUTTON_LEFT），像 C 里用常量做分支判断。
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    // 通过回调获取玩家当前世界坐标；若回调为空则退化为 (0,0)
    Vec2 pos = _getPlayerPos ? _getPlayerPos() : Vec2();
    // Game::globalState() 返回一个全局单例，类似 C 里的全局结构体实例，
    // 这里从中拿到玩家背包指针，用于查询当前选中的工具。
    auto inv = Game::globalState().inventory;
    // 连续判空：先判断背包是否存在，再判断是否有选中工具，
    // 然后检查工具的种类是否为 Sword（枚举值比较）。
    bool swordSelected = inv && inv->selectedTool() && inv->selectedTool()->kind() == Game::ToolKind::Sword;
    if (swordSelected && _monsters && _map) {
        // 静态函数：根据当前的技能树等返回基础伤害数值。
        int baseDamage = Game::Sword::baseDamage();
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
}

} // namespace Controllers
