不要擅自编译运行项目代码。
- 仅允许场景类继承自 SceneBase ；所有业务逻辑仍必须分流到独立模块（Controller/System/Interactor/Manager）。
- SceneBase 统一承担场景骨架与事件转发；具体差异点由子类（GameScene/RoomScene）覆盖指定接口。
- 所有模块之间通过明确接口协作，不允许跨模块私访或内部状态耦合。
SceneBase 职责（唯一允许的场景继承基类）

- 组合与初始化
  - 创建并管理 worldNode 与 PlayerAppearance 。
  - 加载共享 Inventory ，构建 UIController 、 PlayerController 、 GameStateController 。
  - 可选加载 ToolSystem （根据初始化参数启用）。
- 统一事件转发（无业务逻辑）
  - 数字键选择热键栏；滚轮切换热键栏；热键栏点击选择槽位。
  - Z 键作弊（调用 Cheat 模块并刷新 UI；场景本身不直接改库存）。
  - 左键工具触发（启用时转发到 ToolSystem ）。
  - 空格键交互：调用子类覆盖的 onSpacePressed() ；不在场景内处理任何交互逻辑。
- 更新调度
  - update(dt) 只调用控制器的 update() 并刷新提示文案；任何业务逻辑不得进入 update() 。
- 地图挂载
  - 通过 IMapController::addActorToMap(node, zOrder) 将角色加入正确父节点（Farm: TMX；Room: world）。
子类（GameScene / RoomScene）最小职责

- 仅覆盖以下接口以补充差异化特性：
  - createMapController(worldNode) ：返回具体 MapController （Farm/Room）。
  - positionPlayerInitial() ：设置初始位置（农场居中、室内中上部等）。
  - onSpacePressed() ：转发到对应 Interactor ，根据返回动作执行场景切换（农场进屋、室内出屋）。
  - doorPromptText() ：提供门口提示文案（农场“Press Space to Enter House”、室内“Press Space to Exit”）。
- 子类不得新增或处理任何业务逻辑；不得直接操纵 UI 或游戏数据；不得绕过控制器接口。
禁止事项（场景/基类内）

- 不得出现人物属性/行为、工具逻辑、地图碰撞、动画切换、背包操作、存档/加载、任意业务流程。
- 不得新建除 SceneBase 之外的场景继承层次；模块仍采用组合，不得把模块塞进场景。
- 不得在 update() 中写业务逻辑；仅允许控制器 update() 调度与提示刷新。
输入与更新规范

- 键盘：
  - 1–0 ：选择热键栏槽位（通过 UIController ）。
  - Z ：调用 Cheat 模块授予物资并刷新 UI。
  - Space ：SceneBase 先（可选）转发工具使用，再调用子类 onSpacePressed() 。
- 鼠标：
  - 左键：命中热键栏则选择；否则（可选）转发为工具使用。
  - 右键：靠近箱子时转发到 UIController 打开面板。
  - 滚轮：转发到 UIController 切换热键栏选中。
- 更新：
  - SceneBase::update(dt) 只调用 GameStateController::update() 与 PlayerController::update() 并刷新提示文案；不得包含业务逻辑。
模块化与解耦

- 必须通过接口协作：
  - PlayerController 、 ToolSystem 、 UIController 、 GameStateController 、 Interactor 、 MapController 。
  - 房间几何数据（ doorRect/roomRect/bedRect ）仅在 RoomMapController 暴露，禁止从 IMapController 以外访问子类特性除非接口提供。
- 不允许：
  - Controller/System 相互调用对方的私有或内部方法。
  - 任意模块直接修改其他模块内部变量。
  - 场景绕过模块接口直接操作数据。
扩展与新功能引导

- 新功能必须先判定归属模块（例如：合成 → CraftSystem；箱子交互 → ChestController；音效 → AudioManager）。
- 若需场景差异化表现（文案、交互路径），在子类覆盖 doorPromptText() / onSpacePressed() 即可；严禁把逻辑放入场景主体。
合规自检（提交新代码时自动检查）

- 场景文件是否只继承 SceneBase 且仅覆盖四个接口。
- 场景是否只进行模块组合、事件转发与场景切换；无业务逻辑、无数据操作。
- update(dt) 是否仅包含控制器更新与提示刷新。
- 模块间是否通过接口协作，无跨模块私访。
- 接口实现是否集中在对应模块（Controller/System/Interactor）。
代码规模限制

- GameScene.cpp 与 RoomScene.cpp 必须保持不超过 300 行。
- 如超过，立即拆分到对应模块，或将通用部分上移到 SceneBase / 新的模块接口。