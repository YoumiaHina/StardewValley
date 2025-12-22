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
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    Vec2 pos = _getPlayerPos ? _getPlayerPos() : Vec2();
    auto inv = Game::globalState().inventory;
    bool swordSelected = inv && inv->selectedTool() && inv->selectedTool()->kind() == Game::ToolKind::Sword;
    if (swordSelected && _monsters && _map) {
        int baseDamage = Game::Sword::baseDamage();
        Vec2 dir = _getLastDir ? _getLastDir() : Vec2(0, -1);
        std::vector<std::pair<int,int>> tiles;
        Game::Sword::buildHitTiles(_map, pos, dir, tiles, true);
        if (!tiles.empty()) {
            _monsters->applyAreaDamage(tiles, baseDamage);
        }
        return;
    }
}

} // namespace Controllers
