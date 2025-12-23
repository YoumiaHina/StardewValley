#include "Controllers/UI/ToolUpgradePanelUI.h"
#include "Controllers/Systems/ToolUpgradeSystem.h"
#include "Game/WorldState.h"

using namespace cocos2d;

namespace Controllers {

namespace {

// 整个升级面板在默认分辨率上的缩放系数，方便统一调整 UI 大小。
const float UPGRADE_UI_SCALE = 1.2f;

// 把数值等级转换成在面板上显示的文本。
// 这里直接用英文字符串，实际项目中也可以接入本地化系统。
std::string levelText(int level) {
    if (level <= 0) return "Lv 0 (Basic)";
    if (level == 1) return "Lv 1 (Copper)";
    if (level == 2) return "Lv 2 (Iron)";
    return "Lv 3 (Gold)";
}

}

// 构建升级面板的所有节点（只构建一次）。
// 包括：半透明背景、标题文字、每行图标/等级/材料/按钮等。
void ToolUpgradePanelUI::buildPanel() {
    if (_panelNode) return;
    _panelNode = Node::create();
    if (_scene) {
        _scene->addChild(_panelNode, 6);
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto origin = Director::getInstance()->getVisibleOrigin();
        _panelNode->setPosition(Vec2(origin.x + visibleSize.width / 2,
                                     origin.y + visibleSize.height / 2));
    }
    _panelNode->setVisible(false);

    // 背景矩形：使用 DrawNode 画一个半透明黑色矩形。
    auto bg = DrawNode::create();
    float w = 420.f * UPGRADE_UI_SCALE * 2.0f;
    float h = 220.f * UPGRADE_UI_SCALE * 2.0f;
    Vec2 v[4] = { Vec2(-w/2, -h/2), Vec2(w/2, -h/2), Vec2(w/2, h/2), Vec2(-w/2, h/2) };
    bg->drawSolidPoly(v, 4, Color4F(0.f, 0.f, 0.f, 0.85f));
    _panelNode->addChild(bg);

    // 面板标题文字。
    auto title = Label::createWithTTF("Tool Upgrade", "fonts/arial.ttf", 22 * UPGRADE_UI_SCALE);
    if (title) {
        title->setPosition(Vec2(0, h/2 - 26 * UPGRADE_UI_SCALE));
        _panelNode->addChild(title);
    }

    // 预先约定本面板只展示 4 种工具：锄头、镐子、斧头、浇水壶。
    _rows.clear();
    Game::ToolKind kinds[4] = {
        Game::ToolKind::Hoe,
        Game::ToolKind::Pickaxe,
        Game::ToolKind::Axe,
        Game::ToolKind::WaterCan
    };
    float startY = h/2 - 90 * UPGRADE_UI_SCALE;
    float gapY = 70 * UPGRADE_UI_SCALE;
    for (int i = 0; i < 4; ++i) {
        float y = startY - i * gapY;
        RowWidgets row;
        row.kind = kinds[i];

        // 工具图标，占位用的空 Sprite，刷新时会设置具体贴图。
        auto toolIcon = Sprite::create();
        if (toolIcon) {
            toolIcon->setAnchorPoint(Vec2(0.f, 0.5f));
            toolIcon->setPosition(Vec2(-w/2 + 40 * UPGRADE_UI_SCALE, y));
            toolIcon->setVisible(false);
            _panelNode->addChild(toolIcon);
        }
        row.toolIcon = toolIcon;

        // 显示当前工具等级的文字标签。
        auto levelLabel = Label::createWithTTF("", "fonts/arial.ttf", 18 * UPGRADE_UI_SCALE);
        if (levelLabel) {
            levelLabel->setAnchorPoint(Vec2(0.f, 0.5f));
            levelLabel->setPosition(Vec2(-w/2 + 220 * UPGRADE_UI_SCALE, y));
            _panelNode->addChild(levelLabel);
        }
        row.levelLabel = levelLabel;

        // 材料图标：最多显示 5 个小图标，用来描述升级所需材料数量。
        float iconStartX = -w/2 + 380.f * UPGRADE_UI_SCALE;
        float iconGapX = 30.f * UPGRADE_UI_SCALE;
        for (int j = 0; j < 5; ++j) {
            auto icon = Sprite::create();
            if (icon) {
                icon->setAnchorPoint(Vec2(0.5f, 0.5f));
                icon->setPosition(Vec2(iconStartX + iconGapX * static_cast<float>(j), y));
                icon->setVisible(false);
                _panelNode->addChild(icon);
                row.materialIcons.push_back(icon);
            }
        }

        // “升级”按钮：实际上用 Label 充当按钮，并注册触摸监听。
        auto buttonLabel = Label::createWithTTF("[Upgrade]", "fonts/arial.ttf", 18 * UPGRADE_UI_SCALE);
        if (buttonLabel) {
            buttonLabel->setAnchorPoint(Vec2(0.5f, 0.5f));
            buttonLabel->setPosition(Vec2(w/2 - 90 * UPGRADE_UI_SCALE, y));
            buttonLabel->setColor(Color3B::YELLOW);
            _panelNode->addChild(buttonLabel);

            // 事件监听器：使用 EventListenerTouchOneByOne 监听点击事件。
            auto listener = EventListenerTouchOneByOne::create();
            listener->setSwallowTouches(true);
            // lambda 写法：
            // - 捕获列表 [buttonLabel]：把外部的指针按值拷贝到闭包中；
            // - 参数 (Touch* t, Event* e)：与 C 回调类似，只是这里用的是 C++ 对象指针。
            listener->onTouchBegan = [buttonLabel](Touch* t, Event* e) {
                auto target = static_cast<Label*>(e->getCurrentTarget());
                Vec2 p = target->convertToNodeSpace(t->getLocation());
                Size s = target->getContentSize();
                Rect r(0, 0, s.width, s.height);
                if (r.containsPoint(p)) {
                    target->setScale(0.9f);
                    return true;
                }
                return false;
            };
            // onTouchEnded 中捕获 this、kinds、i 和 buttonLabel：
            // - this：允许在回调里访问成员函数 refreshPanel() 和成员变量 _rows；
            // - kinds：值捕获数组，保证回调里看到的仍是当时的 4 种 ToolKind；
            // - i：当前行索引，注意用 int 参与边界检查；
            // - buttonLabel：用于恢复按钮缩放。
            listener->onTouchEnded = [this, kinds, i, buttonLabel](Touch* t, Event* e) {
                buttonLabel->setScale(1.0f);
                if (i < 0 || i >= static_cast<int>(_rows.size())) {
                    return;
                }
                if (!_rows[static_cast<std::size_t>(i)].canUpgrade) {
                    return;
                }
                // 调用系统层的 ToolUpgradeSystem 执行实际的升级逻辑。
                bool ok = ToolUpgradeSystem::getInstance().upgradeToolOnce(_inventory, kinds[i]);
                if (ok) {
                    refreshPanel();
                    if (_onUpgraded) {
                        _onUpgraded();
                    }
                }
            };
            _panelNode->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, buttonLabel);
        }
        row.buttonLabel = buttonLabel;

        _rows.push_back(row);
    }
}

// 根据当前背包/全局箱子里的工具和金钱情况刷新 UI。
// 主要步骤：
// 1. 找到每种工具的当前等级（先查玩家背包，再查全局箱子）；
// 2. 通过 ToolUpgradeSystem::nextUpgradeCost 计算下一次升级所需费用与材料；
// 3. 根据是否可升级更新按钮文本与颜色；
// 4. 更新工具图标与材料图标的贴图和可见状态。
void ToolUpgradePanelUI::refreshPanel() {
    if (!_panelNode) return;
    for (auto& row : _rows) {
        row.canUpgrade = false;
        bool hasTool = false;
        int lv = 0;
        const Game::ToolBase* foundTool = nullptr;
        if (_inventory) {
            std::size_t sz = _inventory->size();
            for (std::size_t i = 0; i < sz; ++i) {
                const Game::ToolBase* t = _inventory->toolAt(i);
                if (t && t->kind() == row.kind) {
                    hasTool = true;
                    lv = t->level();
                    foundTool = t;
                    break;
                }
            }
            if (!hasTool) {
                auto& ws = Game::globalState();
                const Game::Inventory* wsInv = ws.inventory.get();
                if (_inventory.get() == wsInv) {
                    for (const auto& s : ws.globalChest.slots) {
                        if (s.kind == Game::SlotKind::Tool && s.tool && s.tool->kind() == row.kind) {
                            hasTool = true;
                            lv = s.tool->level();
                            foundTool = s.tool.get();
                            break;
                        }
                    }
                }
            }
        }
        if (!hasTool) {
            if (row.levelLabel) {
                row.levelLabel->setString("No Tool");
            }
            if (row.buttonLabel) {
                row.buttonLabel->setString("[No Tool]");
                row.buttonLabel->setColor(Color3B(150, 150, 150));
            }
            if (row.toolIcon) {
                row.toolIcon->setVisible(false);
            }
            for (auto* icon : row.materialIcons) {
                if (icon) icon->setVisible(false);
            }
            continue;
        }
        if (row.levelLabel) {
            row.levelLabel->setString(levelText(lv));
        }
        long long goldCost = 0;
        Game::ItemType materialType = Game::ItemType::CopperIngot;
        int materialQty = 0;
        bool affordable = false;
        bool hasNext = ToolUpgradeSystem::getInstance().nextUpgradeCost(_inventory,
                                                                        row.kind,
                                                                        goldCost,
                                                                        materialType,
                                                                        materialQty,
                                                                        affordable);
        row.canUpgrade = hasNext && affordable;
        if (row.buttonLabel) {
            if (!hasNext) {
                row.buttonLabel->setString("[Max]");
                row.buttonLabel->setColor(Color3B(150, 150, 150));
            } else {
                std::string text = StringUtils::format("[Upgrade %lldG]", goldCost);
                row.buttonLabel->setString(text);
                if (row.canUpgrade) {
                    row.buttonLabel->setColor(Color3B::YELLOW);
                } else {
                    row.buttonLabel->setColor(Color3B(150, 150, 150));
                }
            }
        }
        if (row.toolIcon) {
            if (foundTool) {
                std::string path = foundTool->iconPath();
                if (!path.empty()) {
                    row.toolIcon->setTexture(path);
                }
                if (row.toolIcon->getTexture()) {
                    Size cs = row.toolIcon->getContentSize();
                    float target = 32.f * UPGRADE_UI_SCALE;
                    float sx = cs.width > 0 ? target / cs.width : 1.0f;
                    float sy = cs.height > 0 ? target / cs.height : 1.0f;
                    float s = std::min(sx, sy);
                    row.toolIcon->setScale(s);
                    row.toolIcon->setVisible(true);
                } else {
                    row.toolIcon->setVisible(false);
                }
            } else {
                row.toolIcon->setVisible(false);
            }
        }
        // 根据材料种类选择对应的小图标路径。
        std::string iconPath;
        if (hasNext) {
            switch (materialType) {
                case Game::ItemType::CopperIngot: iconPath = "Mineral/copperIngot.png"; break;
                case Game::ItemType::IronIngot:   iconPath = "Mineral/ironIngot.png"; break;
                case Game::ItemType::GoldIngot:   iconPath = "Mineral/goldIngot.png"; break;
                default: break;
            }
        }
        for (std::size_t idx = 0; idx < row.materialIcons.size(); ++idx) {
            auto* icon = row.materialIcons[idx];
            if (!icon) continue;
            if (!hasNext || iconPath.empty() || idx >= static_cast<std::size_t>(materialQty)) {
                icon->setVisible(false);
                continue;
            }
            icon->setTexture(iconPath);
            if (icon->getTexture()) {
                Size cs = icon->getContentSize();
                float target = 26.f * UPGRADE_UI_SCALE;
                float sx = cs.width > 0 ? target / cs.width : 1.0f;
                float sy = cs.height > 0 ? target / cs.height : 1.0f;
                float s = std::min(sx, sy);
                icon->setScale(s);
                icon->setVisible(true);
            } else {
                icon->setVisible(false);
            }
        }
    }
}

// 控制面板的显示与隐藏：
// - 第一次显示前会确保已经完成 buildPanel；
// - 显示前先刷新内容，避免用旧数据。
void ToolUpgradePanelUI::togglePanel(bool show) {
    if (!_panelNode) buildPanel();
    if (!_panelNode) return;
    if (show) {
        refreshPanel();
        _panelNode->setVisible(true);
    } else {
        _panelNode->setVisible(false);
    }
}

// 简单查询根节点是否存在且处于可见状态。
bool ToolUpgradePanelUI::isVisible() const {
    return _panelNode && _panelNode->isVisible();
}

// 注册一个“升级成功后回调”，由外部（例如 UIController）传入。
void ToolUpgradePanelUI::setOnUpgraded(const std::function<void()>& cb) {
    _onUpgraded = cb;
}

}
