#include "Controllers/MineCombatController.h"
#include "cocos2d.h"
#include "Game/WorldState.h"
#include "Game/Tool/ToolBase.h"

using namespace cocos2d;

namespace Controllers {

void MineCombatController::onMouseDown(EventMouse* e) {
    if (e->getMouseButton() != EventMouse::MouseButton::BUTTON_LEFT) return;
    Vec2 pos = _getPlayerPos ? _getPlayerPos() : Vec2();
    auto inv = Game::globalState().inventory;
    bool swordSelected = inv && inv->selectedTool() && inv->selectedTool()->kind() == Game::ToolKind::Sword;
    int baseDamage = 0;
    if (swordSelected) {
        baseDamage = 8;
    }
    if (swordSelected && _monsters && _map) {
        Vec2 dir = _getLastDir ? _getLastDir() : Vec2(0, -1);
        if (dir.lengthSquared() < 0.001f) dir = Vec2(0, -1);
        int pc = 0, pr = 0;
        _map->worldToTileIndex(pos, pc, pr);
        int dc = 0, dr = 0;
        if (std::abs(dir.x) > std::abs(dir.y)) {
            dc = (dir.x > 0) ? 1 : -1;
        } else {
            dr = (dir.y > 0) ? 1 : -1;
        }
        std::vector<std::pair<int,int>> tiles;
        for (int k = 1; k <= 3; ++k) {
            int tc = pc + dc * k;
            int tr = pr + dr * k;
            if (_map->inBounds(tc, tr)) {
                tiles.emplace_back(tc, tr);
            }
        }
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
