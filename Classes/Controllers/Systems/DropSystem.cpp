#include "Controllers/Systems/DropSystem.h"

namespace Controllers {

void DropSystem::configureTargetProvider(std::function<AttachTarget()> provider) {
    _targetProvider = std::move(provider);
}

void DropSystem::setDrops(const std::vector<Game::Drop>& drops) {
    _drops = drops;
    notifyChanged();
    refreshVisuals();
}

const std::vector<Game::Drop>& DropSystem::drops() const {
    return _drops;
}

void DropSystem::setOnDropsChanged(std::function<void(const std::vector<Game::Drop>&)> cb) {
    _onDropsChanged = std::move(cb);
}

void DropSystem::notifyChanged() {
    if (_onDropsChanged) {
        _onDropsChanged(_drops);
    }
}

void DropSystem::ensureAttached() {
    if (!_targetProvider) return;
    AttachTarget tgt = _targetProvider();
    if (!tgt.parent) return;

    bool parentChanged = (_attachedParent != tgt.parent);
    bool zChanged = (_attachedZOrder != tgt.zOrder);
    _attachedParent = tgt.parent;
    _attachedZOrder = tgt.zOrder;

    if (!_dropsDraw || parentChanged || !_dropsDraw->getParent()) {
        if (_dropsDraw && _dropsDraw->getParent()) {
            _dropsDraw->removeFromParent();
        }
        _dropsDraw = cocos2d::DrawNode::create();
        _attachedParent->addChild(_dropsDraw, _attachedZOrder);
    } else if (zChanged) {
        _dropsDraw->setLocalZOrder(_attachedZOrder);
    }

    if (!_dropsRoot || parentChanged || !_dropsRoot->getParent()) {
        if (_dropsRoot && _dropsRoot->getParent()) {
            _dropsRoot->removeFromParent();
        }
        _dropsRoot = cocos2d::Node::create();
        _attachedParent->addChild(_dropsRoot, _attachedZOrder);
    } else if (zChanged) {
        _dropsRoot->setLocalZOrder(_attachedZOrder);
    }
}

void DropSystem::refreshVisuals() {
    ensureAttached();
    Game::Drop::renderDrops(_drops, _dropsRoot, _dropsDraw);
}

void DropSystem::spawnDropAt(Controllers::IMapController* map, int c, int r, int itemType, int qty) {
    if (!map || qty <= 0) return;
    if (!map->inBounds(c, r)) return;
    Game::Drop d{ static_cast<Game::ItemType>(itemType), map->tileToWorld(c, r), qty };
    _drops.push_back(d);
    notifyChanged();
    refreshVisuals();
}

void DropSystem::collectDropsNear(const cocos2d::Vec2& playerWorldPos, Game::Inventory* inv) {
    if (!inv) return;
    Game::Drop::collectDropsNear(playerWorldPos, _drops, inv);
    notifyChanged();
    refreshVisuals();
}

void DropSystem::clear() {
    _drops.clear();
    notifyChanged();
    if (_dropsDraw) {
        _dropsDraw->removeFromParent();
        _dropsDraw = nullptr;
    }
    if (_dropsRoot) {
        _dropsRoot->removeFromParent();
        _dropsRoot = nullptr;
    }
    _attachedParent = nullptr;
}

} // namespace Controllers

