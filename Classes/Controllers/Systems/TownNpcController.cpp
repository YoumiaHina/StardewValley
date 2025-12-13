#include "Controllers/Systems/TownNpcController.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/UI/UIController.h"
#include "Game/Npc.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace Controllers {

TownNpcController::TownNpcController(TownMapController* map,
                                     Node* worldNode,
                                     UIController* ui,
                                     std::shared_ptr<Game::Inventory> inventory)
    : _map(map), _worldNode(worldNode), _ui(ui), _inventory(std::move(inventory)) {
    buildInitialNpcs();
}

void TownNpcController::buildInitialNpcs() {
    if (!_map || !_worldNode) return;
    Size size = _map->getContentSize();
    float s = _map->tileSize();
    Vec2 base(size.width * 0.5f, size.height * 0.5f);
    const auto& defs = Game::allNpcDefinitions();
    for (std::size_t i = 0; i < defs.size(); ++i) {
        const auto& def = defs[i];
        auto sprite = Sprite::create(def.texturePath);
        if (!sprite) continue;
        float offsetX = (i == 0) ? -2.0f * s : 2.0f * s;
        Vec2 pos(base.x + offsetX, base.y);
        sprite->setPosition(pos);
        _map->addActorToMap(sprite, 22);
        NpcInstance inst;
        inst.id = def.id;
        inst.sprite = sprite;
        _npcs.push_back(inst);
    }
}

bool TownNpcController::findNearestNpc(const Vec2& playerPos,
                                       float maxDist,
                                       Game::NpcId& outId,
                                       Vec2& outPos) const {
    float best = maxDist;
    bool found = false;
    for (const auto& n : _npcs) {
        if (!n.sprite) continue;
        Vec2 local = n.sprite->getPosition();
        float d = playerPos.distance(local);
        if (d < best) {
            best = d;
            outId = n.id;
            Node* parent = n.sprite->getParent();
            Vec2 world = parent ? parent->convertToWorldSpace(local) : local;
            auto cs = n.sprite->getContentSize();
            outPos = Vec2(world.x, world.y + cs.height * 0.5f);
            found = true;
        }
    }
    return found;
}

void TownNpcController::update(const Vec2& playerPos) {
    if (!_ui || !_map) return;
    float maxDist = _map->tileSize() * 1.5f;
    Game::NpcId id;
    Vec2 pos;
    bool isNear = findNearestNpc(playerPos, maxDist, id, pos);
    if (isNear) {
        _ui->showNpcPrompt(true, pos, "Space to Talk / Give Gift");
    } else {
        _ui->showNpcPrompt(false, Vec2::ZERO, "");
    }
}

void TownNpcController::handleTalkAt(const Vec2& playerPos) {
    if (!_ui || !_map) return;
    float maxDist = _map->tileSize() * 1.5f;
    Game::NpcId id;
    Vec2 pos;
    bool isNear = findNearestNpc(playerPos, maxDist, id, pos);
    if (!isNear) return;
    const Game::NpcDefinition* def = Game::npcDefinitionById(id);
    std::string name = def ? def->name : std::string("NPC");
    auto& ws = Game::globalState();
    int key = static_cast<int>(id);
    int current = 0;
    auto it = ws.npcFriendship.find(key);
    if (it != ws.npcFriendship.end()) current = it->second;
    int gained = 5;
    bool usedGift = false;
    if (_inventory && _inventory->size() > 0) {
        if (_inventory->selectedKind() == Game::SlotKind::Item) {
            const auto& slot = _inventory->selectedSlot();
            if (slot.itemQty > 0) {
                gained = Game::npcFriendshipGainForGift(id, slot.itemType);
                bool consumed = _inventory->consumeSelectedItem(1);
                if (consumed) {
                    usedGift = true;
                }
            }
        }
    }
    int next = current + gained;
    if (next > 250) next = 250;
    ws.npcFriendship[key] = next;
    std::string text = name + ": Hello! Friendship " + std::to_string(next);
    _ui->popTextAt(pos + Vec2(0, 24.0f), text, Color3B::WHITE);
}

} // namespace Controllers
