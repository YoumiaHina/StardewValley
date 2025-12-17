不要擅自编译运行项目代码。
- 仅允许场景类继承自 SceneBase ；所有业务逻辑仍必须分流到独立模块（Controller/System/Interactor/Manager）。
- SceneBase 统一承担场景骨架与事件转发；具体差异点由子类（GameScene/RoomScene）覆盖指定接口。
- 所有模块之间通过明确接口协作，不允许跨模块私访或内部状态耦合。

代码注释规范

- 定义一个类（class/struct）时，必须在其头文件中、类定义处前几行用中文注释说明：该类的作用/职责边界/主要协作对象（通过接口而非直接访问内部状态）。
- 编写方法时，每个方法都必须有中文注释说明其作用；注释可写在声明处或定义处（二选一即可），但必须覆盖所有对外接口与关键私有方法。

高内聚低耦合

- 一个类/文件只做一类事情（单一职责）；若同时包含“状态 + 规则 + 渲染 + 输入/流程编排”，必须拆分到对应模块。
- 模块间通过明确接口协作，不允许跨模块私访或依赖对方内部状态；避免把业务规则塞进与其职责无关的类中（例如 MapController/Scene 里堆业务）。

SceneBase 职责（唯一允许的场景继承基类）

代码规模限制

- 任意单个源文件（.h/.cpp）代码行数不超过 1000 行；超出必须拆分（按职责拆到 Controller/System/Interactor/Manager 或上移通用接口）。
- GameScene.cpp 与 RoomScene.cpp 必须保持不超过 300 行。
- 如超过，立即拆分到对应模块，或将通用部分上移到 SceneBase / 新的模块接口。
环境系统与业务“唯一来源”

- 同一类业务实体（矿石、树木、杂草、楼梯等）在项目中必须有且仅有一个“状态唯一来源”模块，通常是某个 *System 或 *Controller （如 MineralSystem ）。
  
  - 禁止在多个模块中维护同一实体的并行状态列表（例如： MineralSystem::_minerals + MineMiningController::_nodes/_minerals 双份存储）。
  - 若发现某模块需要直接访问该实体状态，必须通过唯一来源模块提供的接口获取。
- 所有环境障碍物（矿石、石头、树、草丛等）统一通过 EnvironmentObstacleSystemBase 的子类管理：
  
  - 负责生成、存储、更新、销毁实体状态。
  - 负责挂接和管理对应的渲染节点（sprite / DrawNode 等）。
- 业务 Controller（如 MineMiningController ）禁止自建环境实体容器（ std::vector<MineralData> 之类）来“镜像”系统状态；
  
  - 需要查询或遍历时，通过 MineralSystem 公开接口完成。
  - 需要新增状态字段时，先改动系统层的数据结构，而不是在 Controller 额外塞一份。
视觉节点与 System 的职责划分

- 环境实体对应的 Cocos 节点（例如 Game::Mineral ）的创建、缩放、添加到 worldNode 、位置同步与销毁，必须由对应的 System 完成：
  
  - System 应提供 attachToWorld(Node*) / syncVisuals() / clearVisuals() 等接口。
  - Controller 可以驱动这些接口的调用时机，但禁止在内部 new / create 环境实体节点并长期持有指针。
- Controller 内禁止出现以下模式：
  
  - 维护 _nodes 、 _visuals 等“实体+节点”列表，用以重复管理本应由 System 处理的节点。
  - 在 Controller 自己算瓦片尺寸、贴图缩放，手动挂载到 worldNode 。 正确做法：把这些逻辑集中在 System 内部，让 System 统一为所有调用者提供视觉呈现。
输入转发与交互链路

- 玩家交互（如挖矿、砍树、耕地等）统一链路：
  
  - 输入采集 → PlayerController / ToolSystem → Tool → 对应环境 System（MineralSystem/TreeSystem/RockSystem 等） → 掉落/音效/UI 等其它模块。
  - Tool 必须直接与对应环境 System 交互，不得通过 MapController 的“业务钩子”实现工具逻辑。
  - MapController 只提供必要的 tile/world 坐标转换，以及对环境 System 的统一访问入口（用于 Tool/Controller 获取系统接口），不得夹带工具判定、HP 计算、掉落种类判断等业务规则。
  - 中间的 *Controller（如 MineMiningController）只能做“输入转换 + 调用顺序编排”，不得夹带业务规则（比如 HP 计算、掉落种类判断）。
- *Controller 只关心“有没有命中、有没有打碎、掉了什么”：
  
  - 系统层提供类似 hitAt(worldPos, power) -> HitResult 接口，包含 hit/destroyed/drops 等结果。
  - Controller 根据结果调用 DropSystem 、 InventoryController 、 AudioManager 、 CameraController 等模块，不直接操作矿石/树木的内部数据。
楼层切换与对象生命周期

- 楼层切换 / 地图切换的责任链统一：
  
  - MapController 负责发起： onFloorChanged(floor, candidates, stairPos) 等事件。
  - 各环境 System 负责在自己的实现中清理旧状态与节点，再按新层级生成新状态与节点。
  - Controller 不得在多个地方重复清理同一批节点，避免“多方各持一份指针”的情况。
- 与地图生命周期有关的节点（例如矿石节点、矿物掉落、楼梯节点）必须只挂在一个明确 owner 上：
  
  - MapController 只负责世界节点树结构（TMX、worldNode）和挂载点。
  - 具体实体节点由 System 挂载/移除。
  - 禁止出现 “MapController 删 TMX + 某个 Controller 又自己删一遍子节点” 的双重管理。
新模块/新功能的约束

- 引入新的环境类业务（例如新增一种可破坏的地形、机关）前，必须先判定：
  
  - 是否应实现为某个 EnvironmentObstacleSystemBase 子类，作为该业务的唯一来源。
  - 再决定 Controller 层是否需要新增一个轻量的 *Controller 来做输入转发。
- 在代码评审 / 自检时新增一条检查项：
  
  - “同一实体是否被多处维护状态或节点？”
  - 若存在重复状态/节点容器，应当合并到 System 层，并让其它模块通过接口访问。
