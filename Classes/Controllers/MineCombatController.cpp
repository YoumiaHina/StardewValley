#include "Controllers/MineCombatController.h"
#include "cocos2d.h"
#include "Game/WorldState.h"
#include "Game/Tool/ToolBase.h"

using namespace cocos2d;

namespace Controllers {

void MineCombatController::onMouseDown(EventMouse* e) {
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    Vec2 pos = _getPlayerPos ? _getPlayerPos() : Vec2();
    // 根据当前选中的工具调整伤害（Sword 更高伤害）
    int baseDamage = 20;
    auto inv = Game::globalState().inventory;
    if (inv && inv->selectedTool() && inv->selectedTool()->kind() == Game::ToolKind::Sword) {
        baseDamage = 35;
    }
    // 简化：优先攻击怪物，其次挖掘最近岩石
    if (_monsters) {
        _monsters->applyDamageAt(pos, baseDamage);
    } else if (_mining) {
        _mining->hitNearestNode(pos, /*power*/1);
    }
}

} // namespace Controllers
