#include "Controllers/UI/CraftPanelUI.h"

using namespace cocos2d;

namespace Controllers {

void CraftPanelUI::buildCraftPanel() {
    if (_panelNode) return;
    _panelNode = Node::create();
    if (_scene) _scene->addChild(_panelNode, 5);
}

void CraftPanelUI::refreshCraftPanel(int level) {
    if (!_panelNode) return;
}

void CraftPanelUI::toggleCraftPanel(bool show) {
    if (_panelNode) _panelNode->setVisible(show);
}

}
