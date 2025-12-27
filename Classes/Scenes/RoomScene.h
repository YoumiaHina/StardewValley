/**
 * RoomScene：室内场景（继承 SceneBase），负责室内地图的场景骨架与事件转发。
 * - 职责边界：只做场景初始化/出生点选择/输入事件转发/场景切换，不承载室内业务规则。
 * - 主要协作对象：通过 `Controllers::IMapController` 与室内交互器等接口协作。
 */
#pragma once

#include "cocos2d.h"
#include <memory>
#include <vector>
#include "Game/Inventory.h"
#include "ui/CocosGUI.h"
#include "Game/View/PlayerView.h"
#include "Controllers/Map/IMapController.h"
#include "Controllers/Input/PlayerController.h"
#include "Controllers/Map/RoomMapController.h"
#include "Controllers/UI/UIController.h"
#include "Controllers/Systems/GameStateController.h"
#include "Controllers/Interact/RoomInteractor.h"
#include "Scenes/SceneBase.h"

class RoomScene : public SceneBase {
public:
    // 创建室内场景实例。
    static cocos2d::Scene* createScene();

    
    // 初始化室内场景。
    virtual bool init() override;

    CREATE_FUNC(RoomScene);
    ~RoomScene() override;

    // 设置出生点为门内侧（从农场返回室内时使用）。
    void setSpawnInsideDoor();

private:
    Controllers::RoomMapController* _roomMap = nullptr;
    Controllers::RoomInteractor* _interactor = nullptr;

    // 创建并返回室内地图控制器（以接口形式对 SceneBase 暴露）。
    Controllers::IMapController* createMapController(cocos2d::Node* worldNode) override;

    // 根据当前出生点配置设置玩家初始位置。
    void positionPlayerInitial() override;

    // 处理空格按键行为（通常为交互/工具触发入口）。
    void onSpacePressed() override;

    // 获取门交互提示文本。
    const char* doorPromptText() const override;

protected:
    // 处理键盘按键事件并转发到对应模块。
    void onKeyPressedHook(cocos2d::EventKeyboard::KeyCode code) override;

    // 处理鼠标按下事件并转发到对应模块。
    void onMouseDown(cocos2d::EventMouse* e) override;
};
