#pragma once

#include "Scenes/SceneBase.h"
#include "Controllers/Map/BeachMapController.h"
#include "Controllers/Interact/BeachInteractor.h"
#include "Controllers/NPC/NpcControllerBase.h"
namespace Controllers { class ChestInteractor; }

/**
 * BeachScene：沙滩场景（继承 SceneBase），负责沙滩地图的场景骨架与事件转发。
 * - 职责边界：只做场景初始化/输入事件转发/场景切换触发，不承载沙滩业务规则。
 * - 主要协作对象：通过 `Controllers::IMapController`、交互器与 NPC 控制接口协作。
 */
class BeachScene : public SceneBase {
public:
    // 创建沙滩场景实例。
    static cocos2d::Scene* createScene();
    CREATE_FUNC(BeachScene);

protected:
    // 初始化沙滩场景。
    bool init() override;

    // 创建并返回沙滩地图控制器（以接口形式对 SceneBase 暴露）。
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;

    // 设置玩家在沙滩的初始出生位置。
    void positionPlayerInitial() override;

    // 处理空格按键行为（通常为交互/工具触发入口）。
    void onSpacePressed() override;

    // 获取门交互提示文本。
    const char* doorPromptText() const override;

    // 处理鼠标按下事件并转发到对应模块。
    void onMouseDown(cocos2d::EventMouse* e) override;

private:
    Controllers::BeachMapController* _beachMap = nullptr;
    Controllers::BeachInteractor _interactor;
    Controllers::ChestInteractor* _chestInteractor = nullptr;
    Controllers::NpcController* _npcController = nullptr;
};
