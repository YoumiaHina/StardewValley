// ChestController 实现：负责箱子放置/交互逻辑以及与 WorldState 的同步。
#include "Controllers/Systems/ChestController.h"
#include "Game/WorldState.h"
#include "Game/GameConfig.h"
#include "Game/PlaceableItem/PlaceableItemBase.h"
#include "Game/Tool/ToolFactory.h"
#include "Controllers/Interact/PlacementInteractor.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/Map/RoomMapController.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/Map/BeachMapController.h"
#include "Controllers/UI/UIController.h"
#include <functional>

using namespace cocos2d;

namespace Controllers {

// 挂接到父节点并初始化调试绘制节点（由基类完成）。
// - 这里直接复用 PlaceableItemSystemBase::attachTo 的实现。
void ChestController::attachTo(Node* parentNode, int zOrder) {
    PlaceableItemSystemBase::attachTo(parentNode, zOrder);
}

// 从 WorldState 加载当前场景对应的箱子列表。
// - _isFarm=true  时使用 farmChests；否则使用 houseChests。
void ChestController::syncLoad() {
    auto& ws = Game::globalState();
    if (_isFarm) {
        _chests = ws.farmChests;
    } else {
        _chests = ws.houseChests;
    }
}

// 只读访问箱子列表。
const std::vector<Game::Chest>& ChestController::chests() const {
    return _chests;
}

// 可写访问箱子列表。
std::vector<Game::Chest>& ChestController::chests() {
    return _chests;
}

// 判断给定世界坐标附近是否存在箱子（用于提示/种地避让）。
// - 实现上直接调用 Game::isNearAnyChest，里面使用 PlaceableItemBase::isNearAny。
bool ChestController::isNearChest(const Vec2& worldPos) const {
    return Game::isNearAnyChest(worldPos, _chests);
}

// 判断给定世界坐标是否与任意箱子发生碰撞（用于玩家移动/放置阻挡）。
// - 使用 PlaceableItemBase::collidesAny 统一做“点在矩形内”的判断。
bool ChestController::collides(const Vec2& worldPos) const {
    return Game::PlaceableItemBase::collidesAny<Game::Chest>(
        worldPos,
        _chests,
        [](const Game::Chest& c) { return Game::chestCollisionRect(c); });
}

// 根据当前箱子列表刷新可视节点：优先使用 Chest.png，否则绘制实心矩形。
void ChestController::refreshVisuals() {
    if (!_drawNode) return;
    _drawNode->clear();
    _drawNode->removeAllChildren();
    for (const auto& ch : _chests) {
        auto r = Game::chestRect(ch);
        Vec2 center(r.getMidX(), r.getMidY());
        auto spr = Sprite::create("PlaceableItem/Chest.png");
        if (spr && spr->getTexture()) {
            auto cs = spr->getContentSize();
            if (cs.width > 0 && cs.height > 0) {
                float sx = r.size.width / cs.width;
                float sy = r.size.height / cs.height;
                float scale = std::min(sx, sy);
                spr->setScale(scale);
            }
            spr->setPosition(center);
            _drawNode->addChild(spr);
        } else {
            Vec2 a(r.getMinX(), r.getMinY());
            Vec2 b(r.getMaxX(), r.getMinY());
            Vec2 c(r.getMaxX(), r.getMaxY());
            Vec2 d(r.getMinX(), r.getMaxY());
            Vec2 v[4] = { a, b, c, d };
            _drawNode->drawSolidPoly(v, 4, Color4F(0.6f,0.4f,0.2f,0.9f));
        }
    }
}

// 当前是否应该尝试放置箱子：选中格是箱子物品且数量>0 时返回 true。
bool ChestController::shouldPlace(const Game::Inventory& inventory) const {
    if (inventory.selectedKind() != Game::SlotKind::Item) return false;
    const auto& slot = inventory.selectedSlot();
    return slot.itemType == Game::ItemType::Chest && slot.itemQty > 0;
}

// 放置箱子入口：根据地图类型分发到农场/室内/室外的具体实现。
// - 对 IMapController* 使用 dynamic_cast 判断其真实类型：
//   农场（isFarm=true）、RoomMapController（室内）、其它室外地图。
bool ChestController::tryPlace(Controllers::IMapController* map,
                               Controllers::UIController* ui,
                               const std::shared_ptr<Game::Inventory>& inventory,
                               const Vec2& playerWorldPos,
                               const Vec2& lastDir) {
    if (!map || !ui || !inventory) return false;
    if (map->isFarm()) {
        return placeChestOnFarm(map, ui, inventory, playerWorldPos, lastDir);
    }
    auto* room = dynamic_cast<Controllers::RoomMapController*>(map);
    if (room) {
        return placeChestInRoom(room, ui, inventory, playerWorldPos);
    }
    return placeChestOnOutdoorMap(map, ui, inventory, playerWorldPos);
}

// 与已有箱子交互：透传给 openChestNearPlayer 打开最近的箱子面板。
// - inventory 在此函数中暂未使用，因此通过 (void)inventory 明确标记。
bool ChestController::tryInteractExisting(Controllers::IMapController* map,
                                          Controllers::UIController* ui,
                                          const std::shared_ptr<Game::Inventory>& inventory,
                                          const Vec2& playerWorldPos,
                                          const Vec2& lastDir) {
    (void)inventory;
    return openChestNearPlayer(map, ui, playerWorldPos, lastDir);
}

namespace {

// 将系统内部箱子列表同步回 WorldState 的回调类型。
// - std::function 接口可以绑定任意可调用对象（lambda/函数/仿函数）。
using ChestSyncFunc = std::function<void(const std::vector<Game::Chest>&)>;

// 计算离玩家最近的箱子下标（非农场场景使用距离最近策略）。
// - 通过遍历所有箱子，使用 Vec2::distance 计算真实距离，取最小值。
int nearestChestIndex(const Vec2& playerWorldPos, const std::vector<Game::Chest>& chests) {
    int idx = -1;
    float best = 1e9f;
    for (int i = 0; i < static_cast<int>(chests.size()); ++i) {
        float d = playerWorldPos.distance(chests[static_cast<std::size_t>(i)].pos);
        if (d < best) {
            best = d;
            idx = i;
        }
    }
    return idx;
}

// 通用放置流程：检查可放置、数量上限、同步 WorldState 并扣除背包中的箱子物品。
// - center ：放置位置（世界坐标）。
// - blocked：用于额外阻挡判断的函数对象（例如房间门/床区域）。
// - map/room：用于计算 UI 坐标；两者必有其一非空。
// - syncWorld：把 chs 写回 WorldState 的回调。
bool placeChestCommon(const Vec2& center,
                      Controllers::UIController* ui,
                      const std::shared_ptr<Game::Inventory>& inventory,
                      std::vector<Game::Chest>& chs,
                      const std::function<bool(const Rect&)>& blocked,
                      Controllers::IMapController* map,
                      Controllers::RoomMapController* room,
                      const ChestSyncFunc& syncWorld) {
    if (!ui || !inventory) return false;
    if (inventory->selectedKind() != Game::SlotKind::Item) return false;
    const auto& slot = inventory->selectedSlot();
    if (slot.itemType != Game::ItemType::Chest || slot.itemQty <= 0) return false;

    // 使用 PlaceableItemBase::canPlaceAt 做“接近/阻挡”检查。
    float s = static_cast<float>(GameConfig::TILE_SIZE);
    float margin = s * 0.4f;
    bool okPlace = Game::PlaceableItemBase::canPlaceAt<Game::Chest>(
        center,
        chs,
        [](const Game::Chest& c) { return Game::chestRect(c); },
        blocked,
        margin);
    if (!okPlace) return false;

    if (chs.size() >= static_cast<std::size_t>(Game::Chest::MAX_PER_AREA)) {
        Vec2 uiPos = center;
        if (map) uiPos = map->getPlayerPosition(center);
        else if (room) uiPos = room->getPlayerPosition(center);
        ui->popTextAt(uiPos, "Too many chests", Color3B::RED);
        return false;
    }

    // 在容器中追加一个新 Chest，并设置其位置。
    Game::Chest chest;
    chest.pos = center;
    chs.push_back(chest);

    if (syncWorld) {
        syncWorld(chs);
    }

    inventory->removeItems(Game::ItemType::Chest, 1);
    ui->refreshHotbar();

    Vec2 uiPos = center;
    if (map) uiPos = map->getPlayerPosition(center);
    else if (room) uiPos = room->getPlayerPosition(center);
    ui->popTextAt(uiPos, "Placed Chest", Color3B::YELLOW);
    return true;
}

} // anonymous namespace

// 打开玩家附近的箱子：农场按朝向格子查找，其它场景按距离最近箱子处理。
// - 农场：使用 targetTile(playerWorldPos, lastDir) 得到玩家前方格子。
// - 其它：先要求玩家“接近任意箱子”，再选择距离最近的那个。
bool openChestNearPlayer(Controllers::IMapController* map,
                         Controllers::UIController* ui,
                         const Vec2& playerWorldPos,
                         const Vec2& lastDir) {
    if (!map || !ui) return false;
    auto& chs = map->chests();
    if (chs.empty()) return false;
    int idx = -1;
    if (map->isFarm()) {
        auto tgt = map->targetTile(playerWorldPos, lastDir);
        int tc = tgt.first;
        int tr = tgt.second;
        if (!map->inBounds(tc, tr)) return false;
        Vec2 center = map->tileToWorld(tc, tr);
        for (int i = 0; i < static_cast<int>(chs.size()); ++i) {
            auto r = Game::chestRect(chs[static_cast<std::size_t>(i)]);
            if (r.containsPoint(center)) {
                idx = i;
                break;
            }
        }
    } else {
        if (!Game::isNearAnyChest(playerWorldPos, chs)) return false;
        idx = nearestChestIndex(playerWorldPos, chs);
    }
    if (idx < 0) return false;
    Game::Chest* chest = &chs[static_cast<std::size_t>(idx)];
    ui->buildChestPanel();
    ui->refreshChestPanel(chest);
    ui->toggleChestPanel(true);
    return true;
}

// 打开全局箱子（类似扩展背包）。
// - 使用 WorldState::globalChest 作为持久化存储。
bool openGlobalChest(Controllers::UIController* ui) {
    if (!ui) return false;
    auto& ws = Game::globalState();
    ui->buildChestPanel();
    ui->refreshChestPanel(&ws.globalChest);
    ui->toggleChestPanel(true);
    return true;
}

// 农场放置箱子：通过 PlaceablePlacementBase 选择合适中心点并同步到 farmChests。
bool placeChestOnFarm(Controllers::IMapController* map,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const Vec2& playerPos,
                      const Vec2& lastDir) {
    if (!map || !ui || !inventory) return false;
    Vec2 center;
    bool okCenter = Controllers::PlacementInteractor::selectFarmCenter(map, playerPos, lastDir, center);
    if (!okCenter) return false;
    auto& chs = map->chests();
    // 农场目前不需要额外阻挡区域，因此 blocked 总是返回 false。
    auto blocked = [](const Rect&) { return false; };
    auto& ws = Game::globalState();
    ChestSyncFunc syncWorld = [&ws, map](const std::vector<Game::Chest>& out) {
        ws.farmChests = out;
        if (map) {
            map->refreshMapVisuals();
        }
    };
    return placeChestCommon(center, ui, inventory, chs, blocked, map, nullptr, syncWorld);
}

// 室内放置箱子：禁止挡住门/床区域，并同步到 houseChests。
bool placeChestInRoom(Controllers::RoomMapController* room,
                      Controllers::UIController* ui,
                      std::shared_ptr<Game::Inventory> inventory,
                      const Vec2& playerPos) {
    if (!room || !ui || !inventory) return false;
    Vec2 center;
    bool okCenter = Controllers::PlacementInteractor::selectRoomCenter(room, playerPos, center);
    if (!okCenter) return false;
    auto& chs = room->chests();
    // blocked lambda：若候选矩形与门/床有交集，则视为不允许放置。
    auto blocked = [room](const Rect& r) {
        return r.intersectsRect(room->doorRect()) || r.intersectsRect(room->bedRect());
    };
    auto& ws = Game::globalState();
    ChestSyncFunc syncWorld = [&ws, room](const std::vector<Game::Chest>& out) {
        ws.houseChests = out;
        if (room) {
            room->refreshChestsVisuals();
        }
    };
    return placeChestCommon(center, ui, inventory, chs, blocked, nullptr, room, syncWorld);
}

// 室外地图（Town/Beach）当前不支持放置 Chest，仅占位返回 false。
bool placeChestOnOutdoorMap(Controllers::IMapController* map,
                            Controllers::UIController* ui,
                            std::shared_ptr<Game::Inventory> inventory,
                            const Vec2& playerPos) {
    return false;
}

// 单步转移：背包 -> 箱子指定格。
// - 若背包选中的是物品：栈叠/创建新物品格，由 consumeSelectedItem 驱动数量变化。
// - 若背包选中的是工具：复制工具实例并清空背包格。
// - 每次调用后会把该 Chest 写回 WorldState 对应容器。
void transferChestCell(Game::Chest& chest,
                       int flatIndex,
                       Game::Inventory& inventory) {
    if (flatIndex < 0) return;
    if (chest.slots.empty()) {
        chest.slots.resize(static_cast<std::size_t>(Game::Chest::CAPACITY));
    }
    if (flatIndex >= static_cast<int>(chest.slots.size())) return;
    Game::Slot& cs = chest.slots[static_cast<std::size_t>(flatIndex)];
    int invIndex = inventory.selectedIndex();
    if (invIndex < 0 || invIndex >= static_cast<int>(inventory.size())) return;
    Game::SlotKind invKind = inventory.selectedKind();
    bool invIsItem = inventory.isItem(static_cast<std::size_t>(invIndex));
    bool invIsTool = inventory.isTool(static_cast<std::size_t>(invIndex));
    bool invEmpty = inventory.isEmpty(static_cast<std::size_t>(invIndex));
    Game::ItemStack invStack = inventory.itemAt(static_cast<std::size_t>(invIndex));
    bool chestHasItem = (cs.kind == Game::SlotKind::Item && cs.itemQty > 0);
    bool chestHasTool = (cs.kind == Game::SlotKind::Tool && cs.tool != nullptr);

    // 分支一：选中的是“物品栈”（可叠加）。
    if (invIsItem && invStack.quantity > 0) {
        if (chestHasTool) return;
        if (chestHasItem && cs.itemType != invStack.type) return;
        if (!chestHasItem) {
            cs.kind = Game::SlotKind::Item;
            cs.itemType = invStack.type;
            cs.itemQty = 0;
            chestHasItem = true;
        }
        if (cs.itemQty >= Game::ItemStack::MAX_STACK) return;
        bool ok = inventory.consumeSelectedItem(1);
        if (!ok) return;
        cs.itemQty += 1;
    // 分支二：选中的是工具（不可叠加）。
    } else if (invIsTool) {
        if (chestHasItem || chestHasTool) return;
        const Game::ToolBase* tConst = inventory.toolAt(static_cast<std::size_t>(invIndex));
        if (!tConst) return;
        Game::ToolKind tk = tConst->kind();
        int level = tConst->level();
        cs.kind = Game::SlotKind::Tool;
        cs.tool = Game::makeTool(tk);
        if (cs.tool) {
            cs.tool->setLevel(level);
        }
        cs.itemQty = 0;
        bool cleared = inventory.clearSlot(static_cast<std::size_t>(invIndex));
        if (!cleared) {
            cs.tool.reset();
            cs.kind = Game::SlotKind::Empty;
            cs.itemQty = 0;
            return;
        }
    } else {
        return;
    }

    // 把修改后的 chest 写回 WorldState 中所有箱子列表：
    auto& ws = Game::globalState();
    for (auto& ch : ws.farmChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    for (auto& ch : ws.houseChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    for (auto& ch : ws.townChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    for (auto& ch : ws.beachChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    if (&chest == &ws.globalChest) {
        ws.globalChest = chest;
    }
}

// 背包 -> 箱子：支持 Shift 连续搬运，多次调用 transferChestCell 直到无法再变化。
// - moveAll=true 时，会以 MAX_STACK 为上限循环多次，直到格子或背包状态不再变化。
bool transferInventoryToChest(Game::Chest& chest,
                              int flatIndex,
                              Game::Inventory& inventory,
                              bool moveAll) {
    if (flatIndex < 0) return false;
    if (chest.slots.empty()) {
        chest.slots.resize(static_cast<std::size_t>(Game::Chest::CAPACITY));
    }
    if (flatIndex >= static_cast<int>(chest.slots.size())) return false;

    int invIndex = inventory.selectedIndex();
    if (invIndex < 0 || invIndex >= static_cast<int>(inventory.size())) return false;

    // 最多尝试搬运次数：一次或最多 MAX_STACK 次。
    int maxMoves = moveAll ? Game::ItemStack::MAX_STACK : 1;
    bool movedAny = false;

    for (int i = 0; i < maxMoves; ++i) {
        // 调用前先记录 Chest 槽位与背包格的旧状态，调用后再对比是否有变化。
        Game::Slot beforeChest = chest.slots[static_cast<std::size_t>(flatIndex)];
        Game::ItemStack beforeInvStack = inventory.itemAt(static_cast<std::size_t>(invIndex));
        bool beforeInvEmpty = inventory.isEmpty(static_cast<std::size_t>(invIndex));
        bool beforeInvIsTool = inventory.isTool(static_cast<std::size_t>(invIndex));

        transferChestCell(chest, flatIndex, inventory);

        Game::Slot afterChest = chest.slots[static_cast<std::size_t>(flatIndex)];
        Game::ItemStack afterInvStack = inventory.itemAt(static_cast<std::size_t>(invIndex));
        bool afterInvEmpty = inventory.isEmpty(static_cast<std::size_t>(invIndex));
        bool afterInvIsTool = inventory.isTool(static_cast<std::size_t>(invIndex));

        bool chestSame = (beforeChest.kind == afterChest.kind &&
                          beforeChest.itemType == afterChest.itemType &&
                          beforeChest.itemQty == afterChest.itemQty);
        bool invSame = (beforeInvEmpty == afterInvEmpty &&
                        beforeInvIsTool == afterInvIsTool &&
                        beforeInvStack.type == afterInvStack.type &&
                        beforeInvStack.quantity == afterInvStack.quantity);

        // 若调用前后状态完全相同，则说明已经无法再搬运，提前结束循环。
        if (chestSame && invSame) {
            break;
        }
        movedAny = true;
        if (!moveAll) break;
    }

    return movedAny;
}

// 箱子 -> 背包：支持单个与 Shift 多个搬运，并确保与 WorldState 同步。
// - moveAll=true 时尽量把该箱子格的物品搬满到目标背包格（受栈上限限制）。
bool transferChestToInventory(Game::Chest& chest,
                              int flatIndex,
                              Game::Inventory& inventory,
                              int invIndex,
                              bool moveAll) {
    if (flatIndex < 0 || invIndex < 0) return false;
    if (chest.slots.empty()) {
        chest.slots.resize(static_cast<std::size_t>(Game::Chest::CAPACITY));
    }
    if (flatIndex >= static_cast<int>(chest.slots.size())) return false;
    if (invIndex >= static_cast<int>(inventory.size())) return false;
    Game::Slot& slotChest = chest.slots[static_cast<std::size_t>(flatIndex)];

    bool movedAny = false;

    if (slotChest.kind == Game::SlotKind::Item && slotChest.itemQty > 0) {
        Game::ItemType type = slotChest.itemType;
        bool invEmpty = inventory.isEmpty(static_cast<std::size_t>(invIndex));
        bool invIsItem = inventory.isItem(static_cast<std::size_t>(invIndex));
        Game::ItemStack st = inventory.itemAt(static_cast<std::size_t>(invIndex));
        bool sameType = invIsItem && st.type == type;
        int currentQty = (invEmpty || !invIsItem) ? 0 : st.quantity;
        int space = Game::ItemStack::MAX_STACK - currentQty;
        bool canReceive = (invEmpty || sameType) && (space > 0);
        if (!canReceive) return false;
        int moveCount = moveAll ? std::min(space, slotChest.itemQty) : 1;
        for (int i = 0; i < moveCount; ++i) {
            bool okAdd = inventory.addOneItemToSlot(static_cast<std::size_t>(invIndex), type);
            if (!okAdd) break;
            if (slotChest.itemQty <= 0) break;
            slotChest.itemQty -= 1;
            movedAny = true;
            if (slotChest.itemQty <= 0) {
                slotChest.kind = Game::SlotKind::Empty;
                slotChest.itemQty = 0;
                break;
            }
        }
    } else if (slotChest.kind == Game::SlotKind::Tool && slotChest.tool) {
        bool invEmpty = inventory.isEmpty(static_cast<std::size_t>(invIndex));
        if (!invEmpty) return false;
        auto t = slotChest.tool.get();
        if (!t) return false;
        Game::ToolKind tk = t->kind();
        int level = t->level();
        auto tool = Game::makeTool(tk);
        if (tool) {
            tool->setLevel(level);
        }
        inventory.setTool(static_cast<std::size_t>(invIndex), tool);
        slotChest.tool.reset();
        slotChest.kind = Game::SlotKind::Empty;
        slotChest.itemQty = 0;
        movedAny = true;
    } else {
        return false;
    }

    if (!movedAny) return false;

    auto& ws = Game::globalState();
    for (auto& ch : ws.farmChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    for (auto& ch : ws.houseChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    for (auto& ch : ws.townChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    for (auto& ch : ws.beachChests) {
        if (ch.pos.equals(chest.pos)) {
            ch = chest;
        }
    }
    if (&chest == &ws.globalChest) {
        ws.globalChest = chest;
    }

    return true;
}

}
