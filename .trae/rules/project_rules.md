不要擅自编译运行项目代码。
GameScene.cpp 与 RoomScene.cpp 必须保持极度简洁，任何逻辑不得堆入场景类。
它们只能负责：

创建场景

加载必要模块（组合，而非继承）

管理 update 调度

分发输入事件给控制器

控制场景切换

除此之外，所有逻辑必须按照规则分流到独立模块。

📘 核心架构规则（Trae 必须永久遵守）
Rule 1 — 场景类只负责“场景”，不负责“逻辑”

GameScene / RoomScene 内不得直接出现：

人物属性（HP、体力、金钱等）

人物行为（移动、耕地、砍树）

工具使用逻辑

UI 更新逻辑

TileMap 碰撞检测

动画切换逻辑

背包系统代码

存档/加载逻辑

任意业务流程

这些全部必须抽离到独立类中。

Rule 2 — 所有模块必须独立类化（不得匿名函数、不得塞进 scene）

模块包括：

领域	类名（建议）
角色属性	PlayerStats / PlayerModel
角色行为	PlayerController
工具逻辑	ToolSystem / ToolItem
耕地与地图交互	WorldInteractor / SoilSystem
背包系统	InventoryManager
UI	UIController / StatusBarUI
地图逻辑	MapController
动画	AnimationController / PlayerAnimation
音效	AudioManager
跨场景数据	GameData / DataManager

Trae 必须按照这些领域拆分 / 创建类。

Rule 3 — 模块必须彼此独立，不得互相耦合

不允许：

PlayerController 直接调用 UIController 的内部方法

ToolSystem 修改 MapController 的内部变量

InventoryManager 访问 PlayerController 的私有状态

模块之间沟通必须经过：

明确的接口（public 方法）

或统一的 GameFacade / Context 传递引用

Rule 4 — 所有逻辑不得写进 update()，必须进入对应 Controller

GameScene / RoomScene 的 update() 只能：

void GameScene::update(float dt) {
    playerController->update(dt);
    toolSystem->update(dt);
    mapController->update(dt);
}


任何 实际逻辑 必须在对应模块执行。

Rule 5 — 事件处理必须转发，不得在场景内部处理

比如触摸事件：

bool GameScene::onTouchBegan(...) {
    return playerController->onTouchBegan(...);
}


不允许出现：

// ❌ 直接写逻辑
if (playerPos.x > 100) { ... }

Rule 6 — 当我提交新代码时，你必须自动检查是否违反这套规则

Trae 必须：

自动扫描新提交的代码

判断是否违反上述条款

自动重构并修正不合规之处（或警告我）

Rule 7 — 当添加新功能时，必须先决定属于哪个模块

Trae 必须回答：

这个功能属于哪个系统？

是否需要创建新模块？

是否影响现有 Controller？

之后才能开始修改。

Rule 8 — 永远保持 GameScene / RoomScene 不超过 300 行

如果超过，Trae 必须主动拆分。