#include "Controllers/MineCombatController.h"
#include "cocos2d.h"
#include "Game/WorldState.h"
#include "Game/Tool/ToolBase.h"
#include "Game/Tool/Sword.h"

using namespace cocos2d;

namespace Controllers {

void MineCombatController::onMouseDown(EventMouse* e) {
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    Vec2 pos = _getPlayerPos ? _getPlayerPos() : Vec2();
    auto inv = Game::globalState().inventory;
    bool swordSelected = inv && inv->selectedTool() && inv->selectedTool()->kind() == Game::ToolKind::Sword;
    int baseDamage = 0;
    if (swordSelected) {
        baseDamage = Game::Sword::baseDamage();
    }
    if (swordSelected && _monsters && _map) {
        Vec2 dir = _getLastDir ? _getLastDir() : Vec2(0, -1);
        std::vector<std::pair<int,int>> tiles;
        Game::Sword::buildHitTiles(_map, pos, dir, tiles, true);
        if (!tiles.empty()) {
            _monsters->applyAreaDamage(tiles, baseDamage);
        }
        return;
    }
    if (_monsters && baseDamage > 0) {
        _monsters->applyDamageAt(pos, baseDamage);
    } else if (_mining) {
        _mining->hitNearestNode(pos, 1);
    }
}

} // namespace Controllers
