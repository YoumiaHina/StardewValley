#include "Controllers/AbyssCombatController.h"
#include "cocos2d.h"

using namespace cocos2d;

namespace Controllers {

void AbyssCombatController::onMouseDown(EventMouse* e) {
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    Vec2 pos = _getPlayerPos ? _getPlayerPos() : Vec2();
    // 简化：优先攻击怪物，其次挖掘最近岩石
    if (_monsters) {
        _monsters->applyDamageAt(pos, /*baseDamage*/20);
    } else if (_mining) {
        _mining->hitNearestNode(pos, /*power*/1);
    }
}

} // namespace Controllers
