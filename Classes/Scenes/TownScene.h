#pragma once

#include "Scenes/SceneBase.h"
#include "Controllers/Map/TownMapController.h"
#include "Controllers/Interact/TownInteractor.h"
#include "Controllers/NPC/NpcControllerBase.h"
namespace Controllers { class ChestInteractor; }
namespace Controllers { class AbigailNpcController; class PierreNpcController; }

/**
 * TownScene：城镇场景。
 * - 作用：承载城镇地图的加载、玩家进入与基础交互入口，并把输入/交互转发给业务模块。
 * - 职责边界：只负责场景骨架与事件转发；城镇交互规则由 TownInteractor/NPC/Chest 等模块实现。
 * - 协作对象：通过 TownMapController（IMapController）与 TownInteractor/NpcController/ChestInteractor 等协作。
 */
class TownScene : public SceneBase {
public:
    // 创建场景实例（由引擎切场景调用）。
    static cocos2d::Scene* createScene();
    // Cocos 工厂：创建并初始化对象。
    CREATE_FUNC(TownScene);

protected:
    // 初始化：构建 SceneBase 骨架并接入城镇特有模块。
    bool init() override;
    // 创建城镇地图控制器（IMapController）。
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;
    // 放置玩家初始位置（含跨场景返回点）。
    void positionPlayerInitial() override;
    // 空格交互：转发到城镇交互器（对话/开箱/进门等）。
    void onSpacePressed() override;
    // 门口提示文案。
    const char* doorPromptText() const override;
    // 鼠标交互：转发到交互器/界面模块。
    void onMouseDown(cocos2d::EventMouse* e) override;
    // 按键钩子：转发到城镇自定义快捷键逻辑（若有）。
    void onKeyPressedHook(cocos2d::EventKeyboard::KeyCode code) override;

private:
    Controllers::TownMapController* _townMap = nullptr;
    Controllers::TownInteractor _interactor;
    Controllers::NpcController* _npcController = nullptr;
    Controllers::ChestInteractor* _chestInteractor = nullptr;
};
