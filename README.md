# StardewValley（Cocos2d-x / C++）

本仓库是一个基于 **Cocos2d-x 3.17.2** 的 2D 模拟经营/冒险类小游戏项目（类《星露谷物语》玩法），核心代码位于 `Classes/`，资源位于 `Resources/`，当前主要面向 **Windows（Win32）** 构建与运行。

## 运行与构建（Windows）

### 环境要求

- Visual Studio 2022（安装 “使用 C++ 的桌面开发”）
- 工具集 `v143`（项目文件已指定）

### 运行步骤

1. 用 VS 打开 `proj.win32/StardewValley.sln`
2. 选择配置 `Debug | Win32` 或 `Release | Win32`
3. 生成并运行 `StardewValley`

构建时会将 `Resources/` 复制到输出目录（见 `proj.win32/StardewValley.vcxproj` 的自定义步骤），运行时需保证可执行文件同目录下存在 `Resources/`。

## 游戏内容

### 场景

- 农场：`Classes/Scenes/FarmScene.*`
- 房间（室内）：`Classes/Scenes/RoomScene.*`
- 城镇：`Classes/Scenes/TownScene.*`
- 海滩：`Classes/Scenes/BeachScene.*`
- 矿洞：`Classes/Scenes/MineScene.*`

### 系统/模块（Controller / System）

- 作物：`Classes/Controllers/Systems/CropSystem.*`
- 动物：`Classes/Controllers/Systems/AnimalSystem.*`
- 天气：`Classes/Controllers/Systems/WeatherController.*`
- 节日：`Classes/Controllers/Systems/FestivalController.*`
- 钓鱼：`Classes/Controllers/Systems/FishingController.*`
- 捏脸/角色外观：`Classes/Scenes/CustomizationScene.*` + `Classes/Game/View/PlayerView.*`
- 商店：`Classes/Controllers/Store/StoreController.*` + `Classes/Controllers/UI/StorePanelUI.*`
- NPC / 社交 / 对话：`Classes/Controllers/NPC/*` + `Classes/Controllers/UI/NpcSocialPanelUI.*` + `Classes/Controllers/UI/DialogueUI.*`
- 合成/配方：`Classes/Controllers/Crafting/CraftingController.*` + `Classes/Game/Recipe/*`
- 工具升级：`Classes/Controllers/Systems/ToolUpgradeSystem.*`
- 可放置物（箱子等）：`Classes/Controllers/Systems/PlaceableItemSystemBase.h` + `Classes/Controllers/Systems/ChestController.*` + `Classes/Game/PlaceableItem/*`
- 掉落：`Classes/Controllers/Systems/DropSystem.*`
- 音乐/音频：`Classes/Controllers/Managers/AudioManager.*`
- 战斗/怪物（矿洞）：`Classes/Controllers/Mine/*` + `Classes/Game/Monster/*`
- 环境障碍物（树/石头/矿石/杂草/楼梯等）：`Classes/Controllers/Environment/*`
- UI 聚合入口：`Classes/Controllers/UI/UIController.*`（统一管理 HUD/背包/面板等）

## 玩法系统简述（后续补充对应的贴图……）

### 商店系统

- 城镇杂货店可购买种子/基础物资，也可出售农产品、矿物等换取金币。
- 农场动物商店可购买鸡/牛/羊等动物，购买后需要在农场选择位置放置。
- 买卖会影响金币与背包物品数量；部分购买行为也会消耗精力。
- 商店面板支持分类与分页，便于快速浏览与交易。

### 农场管理（作物 / Crop）

- 用锄头开垦土地后播种，随后使用水壶浇水推进生长。
- 作物在每天结算时成长；缺水会停滞或有概率枯死，换季不适合作物也会枯死。
- 成熟后可收获获得农产品；背包满时会以掉落物形式留在地面。
- 睡觉进入下一天会触发作物推进，并重置当天“已浇水”的状态。

### 农场管理（动物 / Animal）

- 购买动物后会在农场游走，玩家可以靠近进行喂食。
- 喂食影响动物当日状态，并与每日产物（鸡蛋/牛奶/羊毛等）结算相关。
- 每天睡觉后结算成长与产物；不在农场时产物会延迟到回到农场后出现为掉落物。

### 资源采集与环境障碍物

- 地图中分布树木、石头、矿石、杂草等可破坏/可采集的环境物。
- 使用对应工具进行采集，获得木材/石头/矿物/纤维等资源掉落。
- 掉落物可在地面拾取进入背包，用于后续合成、升级与交易。

### 矿洞冒险（采矿）

- 进入矿洞后，每层都会生成矿石/石头/硬石等资源点，分布在可探索区域，各资源点的出现概率和所需敲击次数不同。
- 使用镐子等工具敲击矿点，可获得矿石、宝石与石头等掉落，部分矿点可能刷新隐藏楼梯。
- 楼层越深，矿物种类与稀有度越高，产出的矿石可以送往熔炉冶炼为金属锭，用于合成与工具升级，也可以售卖换钱。
- 采矿会持续消耗精力，需要在矿洞中合理安排路线，或返回农场休息补给。

### 矿洞冒险（战斗）

- 每层都会刷新对应强度的怪物，每种怪物伤害、血量、速度、动画等属性都不同，玩家可使用武器进行攻击。
- 击败怪物可获得金币、经验与战利品掉落，部分怪物也可能掉落楼梯或特殊钥匙类道具。
- 通过楼梯向下推进楼层，每过5层会解锁一级电梯并获赠新的剑，电梯允许快速前往已解锁楼层，减少重复跑图。
- 战斗过程中需要同时关注生命值与精力，合理使用食物/药品，并在必要时撤退返回农场存档。

### 可放置物系统（箱子 / 熔炉）

- 玩家可以在农场、房屋室内等场景放置箱子，作为额外的储物空间。
- 靠近箱子并交互可打开箱子面板，在背包与箱子之间自由搬运物品；箱子内容会随存档长期保留。
- 熔炉同样属于可放置物，放置后可以向其中投入矿石与燃料，启动熔炼计时。
- 熔炼完成后会在熔炉附近生成金属锭等掉落物，玩家拾取后进入背包，用于后续合成与工具升级。
- 箱子与熔炉的放置都遵循统一的占位与碰撞规则，避免阻挡玩家必经路径或重叠放置。

### 工具升级系统

- 工具（锄头、镐子、斧头、浇水壶）拥有多级品质，等级越高，效率和作用范围越好。
- 通过工具升级面板，玩家可以查看当前每种工具的等级以及下一次升级所需的金币与金属锭材料。
- 若背包和全局箱子中的材料与金币满足要求，点击升级按钮将消耗对应资源，并立刻提升工具等级（最高 3 级）。
- 升级成功后会刷新背包与热键栏显示，使高等级工具在采矿、伐木、耕地与浇水等操作中体现明显差异。

### 时间 / 天气 / 日结

- 游戏时间持续流逝，驱动一天的行动节奏；睡觉进入下一天并触发各类每日结算。
- 天气以晴天/雨天为主：雨天会改变氛围与移动手感，并自动视为“已浇水”。
- 特定节日当天保证晴天，避免活动受到天气影响。

### 社交系统（NPC / 对话 / 好感度）

- 城镇/农场/海滩分布不同 NPC，可通过对话了解剧情与触发交互选项。
- 好感度会随对话与赠礼提升；赠礼存在“每日一次”的限制，避免无限刷好感。
- 社交面板可查看 NPC 的基础信息、好感与关系状态。
- 节日会在特定日期开启，城镇/海滩出现活动布置与特殊交互内容。

### 钓鱼系统（Fishing）

- 在水边使用钓鱼竿开始钓鱼小游戏，通过按住/松开控制捕获条位置。
- 捕获条与鱼影重叠越多，进度增长越快；进度满则钓鱼成功并获得鱼与经验。
- 特定节日提供“节日钓鱼”玩法与限定鱼种奖励。

### 音乐系统（BGM）

- 不同场景会播放不同的背景音乐，切换场景时自动切歌以增强氛围。
- 音频播放避免重复触发与突兀切换，保证整体体验稳定流畅。

## 操作说明（默认按键）

- 移动：`WASD` / 方向键（按住一段时间会自动加速）
- 工具使用：
  - 农场/城镇等：`Space`（场景可配置是否启用）
  - 矿洞：鼠标左键（`MineScene` 默认启用左键工具）
- NPC / 社交：
  - `Space`：与附近 NPC 交互（对话 / 赠礼，取决于当前是否手持可赠送物品）
  - 鼠标右键点击 NPC：打开社交面板（部分 NPC）/ 直接打开商店（如 Pierre）
- 对话框：
  - 鼠标点击对话框区域：推进下一句（无选项时）
  - 鼠标点击选项按钮：选择分支（有选项时）
- 物品栏：
  - 切换选中：鼠标滚轮 / 数字键 `1`~`0`
- 面板：
  - `C`：合成面板
  - `L`：技能面板
  - `Esc`：关闭当前面板；无面板时会保存并返回主菜单
- 存档：
  - `F5`：快速保存
- 其它：
  - `F`：食用当前选中可食用物品
  - `Q`：丢弃/放置当前选中物品（与地图/选中物品类型有关）
  - `Ctrl`：工具距离修饰键（仅对满级工具生效，按住时把单格锄地/浇水/采集改为扇形多格范围）
  - `Shift`：在箱子界面按住再点击物品格，可连续搬运整叠物品

说明：项目内还包含少量开发用快捷键（例如 `Z` 发放基础物资、`T/X` 时间/作物推进等），以 `Classes/Controllers/Input/PlayerController.cpp` 为准。

## 存档位置

存档通过 `Classes/Game/Save/SaveSystem.*` 写入文本文件，默认目录为 Cocos 的可写目录下的 `save/` 子目录（通过 `FileUtils::getInstance()->getWritablePath()` 计算），并支持 `save1.txt` ~ `save50.txt` 槽位命名。

## 技术架构

- 场景骨架：所有场景继承 `SceneBase`，集中完成 worldNode/玩家视图/背包/UI/通用控制器的创建与 update 调度。
- 数据唯一来源：全局状态集中在 `Game::WorldState`（背包、地图瓦片、作物/动物、NPC 好感度等）并通过 `globalState()` 统一读写。
- 地图抽象：场景通过 `Controllers::IMapController` 屏蔽不同 TMX 地图差异，提供坐标转换、碰撞、掉落拾取、节日图层开关等接口。
- 输入与玩法链路：输入采集集中在 `PlayerController`，由“背包选中工具 -> Tool 执行 -> 对应 System/Controller 处理”完成业务闭环；场景只做事件转发与调用顺序编排。
- 存档与版本兼容：`SaveSystem` 以版本号增量读取字段，解析失败时通过 `istream` 状态与边界检查尽量保证存档可恢复。

## 项目特色

### （1）版本控制与协作

- 提交记录：当前仓库累计约 `270` 次提交（`git rev-list --count HEAD`），以小步迭代方式推进功能与修复。
- 分工协作：不同作者围绕 UI、地图、玩法系统、注释与文档分别提交，常见合并提交体现了并行开发后的集成节奏。
- 记录可追溯：关键玩法迭代与问题修复在提交信息中留有明确描述，例如“钓鱼节与天气冲突修复”“鱼速度 bug 修复”等，便于回溯定位。

### （2）代码质量

- 单元测试：游戏工程自身（`Classes/`）未发现接入 `gtest/catch2` 等单测框架与 `TEST(...)` 用例；引擎侧提供 `cocos2d/tests/cpp-tests` 作为功能验证与参考。
- 异常处理：整体偏“显式返回值 + 防御式检查”风格，关键流程以早退避免非法状态（例如工具升级 ；存档读取 。
- 内存管理：大量节点生命周期交由 Cocos 场景树管理，同时在模块析构中主动清理自己创建的节点/对象（例如 `UIController` 释放子 UI 对象，矿洞怪物控制器移除 sprite）。同时也存在部分 `new` 出来的控制器未在 `SceneBase` 中显式 `delete` 的情况，更严格的做法是补齐统一 owner 或改为 `std::unique_ptr` 托管。

### （3）开发特性

- C++11/14 特性：合理使用 lambda 回调做模块解耦与事件绑定（例如钓鱼竿启动回调绑定）；单例采用函数内 `static`（线程安全）初始化。
- 架构清晰：SceneBase 只承担“骨架 + 调度 + 转发”，业务逻辑下沉到 `Controllers/*` 与 `Game/*`，以接口对接减少跨模块耦合。
- 目录结构明确：`Classes/Scenes` 放场景骨架；`Classes/Controllers` 放控制器/系统；`Classes/Game` 放数据结构、道具与规则定义；`Resources` 放地图/贴图/音乐。

### （4）界面与体验

- UI 聚合：`UIController` 作为统一入口管理 HUD/热键栏/背包与各类面板，避免 UI 分散在多个系统中互相直接引用。
- 交互一致：社交/对话/钓鱼/箱子/商店等面板打开时统一阻止移动，减少输入冲突。

## 类的架构

本项目在代码层面整体遵循“场景骨架 + 控制器/系统 + 游戏数据 + UI”的分层设计，尽量做到高内聚、低耦合。下面从几个主要维度简要说明类的架构与协作关系。

### 场景层（Scenes）

- 场景基类：`Classes/Scenes/SceneBase.*` 是所有具体场景的唯一基类，负责：
  - 创建并持有通用控制器（地图、玩家、作物、天气、节日、钓鱼、UI 等）。
  - 搭建 worldNode / UI 根节点等挂载点。
  - 在 `update` 中统一驱动各控制器与系统的帧更新。
- 具体场景：`FarmScene`、`RoomScene`、`TownScene`、`BeachScene`、`MineScene` 等（位于 `Classes/Scenes/*`）仅负责：
  - 指定使用的地图控制器实现（农场/城镇/矿洞 TMX 等）。
  - 根据场景需求按需创建额外的控制器或系统（如矿洞战斗、电梯、怪物控制器等）。
  - 覆盖少量与该场景强相关的钩子（例如场景进入/离开时的初始化或清理）。
- 设计原则：所有业务规则（掉落计算、战斗数值、作物成长等）不直接写在场景类中，而是下沉到 Controllers/System/Game 层，场景只承担“骨架 + 调度 + 事件转发”的职责。

**关键类示例（`Classes/Scenes`）**

- `SceneBase`：场景骨架与调度中心，持有通用控制器与 world/UI 根节点。
- `FarmScene`：农场室外场景，组合 `FarmMapController`、作物/动物系统并挂接农场专用 UI。
- `RoomScene`：室内/房间场景，负责与房间地图控制器、室内交互器协作。
- `TownScene`：城镇场景，聚合 NPC/商店/节日等与 `TownMapController` 的交互。
- `BeachScene`：海滩场景，承载钓鱼、节日活动等，与海滩地图控制器协作。
- `MineScene`：矿洞场景，组合 `MineMapController`、矿洞战斗/怪物/电梯等控制器。

### 控制器层（Controllers / Interactors / Managers）

控制器层负责“流程编排与输入转发”，是场景与底层系统之间的中介。

- 地图控制器：
  - 抽象接口：`Controllers::IMapController` 位于 `Classes/Controllers/Map/*`，统一封装 TMX 地图加载、坐标转换、碰撞检测、地块属性查询、楼层切换等能力。
  - 具体实现：`FarmMapController`、`TownMapController`、`MineMapController` 等分别服务于不同场景，但都通过 `IMapController` 暴露统一接口，场景与系统只依赖抽象。
- 玩家与输入控制器：
  - `PlayerController`（`Classes/Controllers/Input/PlayerController.*`）集中处理键盘/鼠标输入、移动、工具使用触发等，将“玩家按键”转换为对 Tool / System 的调用。
  - 与背包、UI 交互：通过接口与背包系统、UI 面板协作，而不直接操作底层容器。
- 玩法控制器与交互器：
  - 例如 `Classes/Controllers/Mine/*` 下的一组控制器/交互器负责矿洞战斗、电梯、宝箱、楼层事件等，它们更多做“命中结果 → 调用 System/Drop/UI”的调用顺序编排。
  - 合成、商店、社交等模块也有各自的 Controller/Interactor（如 `CraftingController`、`StoreController`、各类 *Interactor），负责把 UI 操作或输入事件翻译成对系统层的调用。
- 管理类（Manager）：
  - 典型如 `AudioManager`（`Classes/Controllers/Managers/AudioManager.*`），集中管理 BGM/音效的播放、切换与资源复用，对外提供简单接口（播放/停止/切换），内部自行维护状态。
  - 其他 Manager 类也遵循类似模式：提供清晰接口，内部隐藏具体实现与资源管理。

**关键类示例（`Classes/Controllers`）**

- `PlayerController`（Input）：负责采集玩家输入、移动与工具使用触发，是“输入入口”。
- `UIController`（UI）：统一创建和管理 HUD/背包栏/各类面板，是 UI 的聚合入口。
- `StoreController`（Store）：处理商店买卖逻辑，与 `StorePanelUI`、`WorldState` 协作修改金币与物品。
- `CraftingController`（Crafting）：处理合成配方查询与合成结果生成，驱动背包与合成 UI。
- `WeatherController` / `FestivalController`（Systems）：围绕 `WorldState` 驱动天气变化与节日开关。
- `FishingController`（Systems）：管理钓鱼小游戏流程，与 `PlayerController` 输入与鱼行为/掉落系统协作。
- `MineMapController` / `FarmMapController` / `TownMapController`（Map）：封装各自场景的 TMX 地图与碰撞/坐标转换。
- 矿洞相关控制器（`Classes/Controllers/Mine/*`）：
  - 典型如战斗控制器、怪物控制器、电梯控制器、矿洞交互器等，负责把命中结果分发给 `DropSystem`、经验系统与 UI。

### 系统层（Systems）

系统层主要负责“状态唯一来源”和“规则实现”，对同一类业务实体提供集中管理。

- 作物与动物：
  - `CropSystem`（`Classes/Controllers/Systems/CropSystem.*`）管理所有地块作物生长状态，是作物相关数据的唯一来源，负责每日推进、浇水状态、成熟与枯死等规则。
  - `AnimalSystem`（`Classes/Controllers/Systems/AnimalSystem.*`）集中管理农场动物的状态、产物结算等。
- 时间、天气与节日：
  - `WeatherController` 与 `FestivalController`（位于 `Classes/Controllers/Systems/`）共同围绕 `WorldState` 驱动每日天气变化、节日开启与关闭等。
  - 这些控制器更多承担“系统级规则”，与具体场景解耦。
- 工具与升级：
  - `ToolUpgradeSystem`（`Classes/Controllers/Systems/ToolUpgradeSystem.*`）负责工具升级消耗、条件校验与升级结果写回，避免把升级规则散落在 UI 或场景中。
- 掉落与可放置物：
  - `DropSystem`（`Classes/Controllers/Systems/DropSystem.*`）负责统一生成、管理与回收地图上的掉落实体，避免各业务模块各自维护掉落列表。
  - `PlaceableItemSystemBase` / `ChestController` 等处理可放置物体（如箱子）的状态与与挂载节点。
- 环境障碍物系统：
  - 所有树木、石头、矿石、杂草、楼梯等环境实体由 `EnvironmentObstacleSystemBase` 的子类统一管理（目录 `Classes/Controllers/Environment/*`），例如矿石相关的 `MineralSystem`。
  - 这些 System 既是状态唯一来源，也是对应 Cocos 节点（sprite/DrawNode）的唯一 owner，负责生成、更新与销毁节点，并通过接口向控制器暴露命中/破坏/掉落等结果。

**关键类示例（`Classes/Controllers/Systems` 与 `Environment`）**

- `CropSystem`：唯一管理作物状态与可视节点，负责每日推进、浇水/枯死/成熟等规则。
- `AnimalSystem`：集中管理动物状态、喂食与产物结算，并驱动对应节点更新。
- `ToolUpgradeSystem`：处理工具升级条件、消耗与结果写回（更新工具等级与属性）。
- `DropSystem`：统一管理掉落物生成/更新/销毁和地图挂载，是掉落状态唯一来源。
- `PlaceableItemSystemBase`：可放置物体（如箱子）的通用系统基类。
- `ChestController`：基于 `PlaceableItemSystemBase` 管理箱子放置、打开/关闭与存取物。
- `EnvironmentObstacleSystemBase`：环境障碍物系统抽象基类，统一定义生成/更新/销毁接口。
- `MineralSystem` 等环境子系统：分别管理矿石、树木、石头、杂草等具体环境实体。

### 游戏数据与规则层（Game）

`Classes/Game/*` 目录下主要是与具体引擎无关的游戏数据结构与规则定义，尽量保持“无场景、无 UI、弱引擎依赖”：

- 全局状态：`Game::WorldState` 作为全局游戏状态的唯一来源，包含背包、地图瓦片、作物/动物、NPC 好感度、时间/天气等核心数据，通过 `globalState()` 访问。
- 道具与配方：`Classes/Game/Item/*`、`Classes/Game/Recipe/*` 定义了物品基础属性、堆叠规则、合成配方等。
- 生物与战斗：`Classes/Game/Monster/*`、玩家与怪物的数值结构等，描述生命值、攻击力、经验、掉落表等战斗相关规则。
- 存档：`Classes/Game/Save/SaveSystem.*` 负责将 `WorldState` 读写到磁盘，并处理版本演进与兼容。

**关键类示例（`Classes/Game`）**

- `Game::WorldState`：全局状态唯一来源，持有背包、时间、季节、天气、地图瓦片、作物/动物、NPC 状态等。
- 物品相关：
  - 基础物品/堆叠结构（如物品 ID、数量、品质等）用于背包与掉落。
  - 合成配方/配方表（`Recipe` 等）描述输入→输出的规则。
- 怪物与战斗相关数据结构：定义怪物基础属性、掉落表、经验与战斗行为参数。
- `SaveSystem`：负责把 `WorldState` 序列化/反序列化到文本存档，并处理版本兼容。

### UI 层（UI）

UI 层以 `UIController` 为核心入口，将各个独立 UI 面板组织成一个整体：

- UI 聚合入口：
  - `UIController`（`Classes/Controllers/UI/UIController.*`）在场景创建时由 `SceneBase` 创建并持有，负责初始化 HUD/背包栏/提示气泡/各类面板，并与 `PlayerController`、各 System 协作刷新显示。
- 具体 UI 面板：
  - 商店 UI：`StorePanelUI`（`Classes/Controllers/UI/StorePanelUI.*`）。
  - 社交 UI：`NpcSocialPanelUI`、`DialogueUI` 等。
  - 其他如技能、合成、箱子等面板也各自有独立类，遵循“单一面板单一类”的原则，通过 `UIController` 或对应 Controller 进行打开/关闭与数据刷新。
- 与系统协作方式：
  - UI 层不直接修改底层数据结构，而是通过 Controller/System 提供的接口完成购买、制作、赠礼、取物等操作；UI 只负责展示与输入采集。

**关键类示例（`Classes/Controllers/UI`）**

- `UIController`：全局 UI 管理入口，持有 HUD、背包栏及各类面板的实例。
- `StorePanelUI`：商店 UI，展示可购买/可出售物品，并与 `StoreController` 协作完成交易。
- `NpcSocialPanelUI`：社交面板 UI，展示 NPC 基本信息与好感度。
- `DialogueUI`：对话框 UI，负责台词显示与选项交互。
- 其他如技能面板、合成面板、箱子面板等，各自对应独立 UI 类，由 `UIController` 或相关 Controller 负责打开/关闭与数据刷新。

### 作物（Crop）

- 状态唯一来源：`CropSystem`（`Classes/Controllers/Systems/CropSystem.*`）持有作物运行时列表并同步写回 `WorldState`，对外仅暴露“种植/浇水/推进/收获”等接口。
- 静态定义与映射：`Game::CropBase` / `Game::CropDefs`（`Classes/Game/Crops/crop/CropBase.*`）提供阶段天数、季节适配、回生属性，以及“种子/产物”物品映射。
- 交互入口：
  - 播种：`FarmInteractor`（`Classes/Controllers/Interact/FarmInteractor.*`）在玩家选中种子时调用 `CropSystem::plantCrop`，并触发地图作物可视刷新。
  - 收获：`Hoe`（`Classes/Game/Tool/Hoe.*`）与 `CropSystem::harvestByHoeAt` 协作计算产物与数量（含技能树加成），再由上层将结果落入背包或掉落系统。
- 日结推进：`CropSystem::advanceCropsDaily` 在“睡觉进入下一天”时被调用，统一处理浇水复位、阶段推进、枯死与回生等分支。

**关键类示例（Crop）**

- `CropSystem`：作物规则与状态唯一来源，统一承载种植/推进/收获。
- `Game::Crop` / `Game::CropType`：作物运行时状态与类型枚举。
- `Game::CropDefs` / `Game::CropBase`：作物静态定义与类型→规则查询入口。
- `FarmInteractor`：把“种子选中 + 交互输入”翻译为作物系统调用。

### 动物（Animal）

- 状态唯一来源：`AnimalSystem`（`Classes/Controllers/Systems/AnimalSystem.*`）维护动物运行时列表（位置/目标/成长/喂食）并同步写回 `WorldState::farmAnimals`，同时作为精灵节点与状态标签的唯一 owner。
- 静态定义：
  - 数据结构：`Game::AnimalType` / `Game::Animal` 与 `Game::animalPrice` 位于 `Classes/Game/Animals/Animal.h`。
  - 行为常量：`Game::AnimalBase`（`Classes/Game/Animals/AnimalBase.h`）的派生类（`ChickenAnimal`/`CowAnimal`/`SheepAnimal`）提供速度、游走半径与贴图路径，系统侧按 `AnimalType` 索引获取。
- 交互入口：
  - 喂食：`FarmInteractor` 调用 `AnimalSystem::tryFeedAnimal` 只返回“建议消耗数量”，背包扣除仍由上层完成以避免系统耦合背包细节。
  - 购买：动物商店面板 `AnimalStorePanelUI`（`Classes/Controllers/UI/StorePanelUI.*`）通过回调驱动 `AnimalSystem::buyAnimal`（价格通常来自 `Game::animalPrice`）扣费并生成；面板的打开入口由 NPC 交互接口 `NpcControllerBase`（`Classes/Controllers/NPC/NpcControllerBase.*`）触发。
- 日结推进：`advanceAnimalsDaily`（`Classes/Controllers/Systems/AnimalSystem.*`）统一处理成长结算与当日产物生成（落地到掉落系统或写入离线掉落）。

**关键类示例（Animal）**

- `AnimalSystem`：动物状态唯一来源，统一管理游走/喂食/产物与可视节点生命周期。
- `Game::Animal` / `Game::AnimalType` / `Game::animalPrice`：动物数据结构与定价（`Classes/Game/Animals/Animal.h`）。
- `Game::AnimalBase` 与 `ChickenAnimal`/`CowAnimal`/`SheepAnimal`：动物静态行为定义（贴图/速度/游走半径）。
- `advanceAnimalsDaily`：动物每日结算入口（成长 + 产物生成）。
- `AnimalStorePanelUI`：动物购买 UI，使用回调解耦扣费与生成逻辑。

### 怪物（Monster）

- 状态唯一来源：`MineMonsterController`（`Classes/Controllers/Mine/MonsterSystem.*`）维护矿洞内所有怪物的运行时列表（位置/速度/当前 HP/攻击冷却），并负责与 `MineMapController` 同步碰撞体和与世界节点同步精灵。
- 静态行为定义：`Game::MonsterBase` 及其派生类（`GreenSlime` / `BlueSlime` / `RedSlime` / `Bug` / `Ghost` 等，位于 `Classes/Game/Monster/*`）提供每种怪物的基础属性（血量/攻击/移动速度/搜敌范围）和动画接口，通过 `monsterInfoFor(MonsterType)` 统一查询。
- 交互入口：
  - 近战攻击：`MineCombatController`（`Classes/Controllers/Mine/CombatSystem.*`）在鼠标左键按下时读取当前选中的 `Game::Sword`，通过 `Game::Sword::buildHitTiles` 构建前方扇形瓦片列表，并调用 `MineMonsterController::applyAreaDamage` 结算怪物伤害与死亡（经验/掉落由怪物系统内部通过 `SkillTreeSystem`、`DropSystem` 等模块完成）。
  - 怪物反击：`MineMonsterController::update` 每帧根据玩家位置驱动怪物移动与接触伤害，更新玩家生命值写回 `WorldState`，并在玩家死亡时清空怪物。

**关键类示例（Monster）**

- `MineMonsterController`：怪物运行时状态唯一来源，负责刷怪/移动/攻击/死亡。
- `Game::MonsterBase` / `monsterInfoFor`：怪物静态配置与动画接口。
- `GreenSlime` / `BlueSlime` / `RedSlime` / `Bug` / `Ghost`：具体怪物行为实现。
- `MineCombatController`：把玩家攻击输入翻译为对怪物系统的范围伤害调用。

### 矿物与矿脉（Mineral）

- 状态唯一来源：`MineralSystem`（`Classes/Controllers/Environment/MineralSystem.*`）作为矿洞内“矿石/石块”环境障碍的唯一来源，持有 `std::vector<Game::MineralData>` 作为运行时列表，并维护 `_obstacles` 映射以同步到 Cocos 节点。
- 运行时数据与静态定义：
  - 运行时：`Game::MineralData`（`Classes/Game/EnvironmentObstacle/Mineral.h`）记录每一块矿石的类型、位置、HP、尺寸与贴图路径。
  - 静态：`Game::MineralType` 枚举区分普通石头/硬石/巨岩/铜矿/铁矿/金矿，`Game::mineralDropItem` 负责把矿石类型映射为掉落物品（石头或各类矿粒）。
- 生成与刷新：
  - 生成：`MineralSystem::generateNodesForFloor` 根据 `MineMapController` 提供的候选点与楼层信息，使用随机数决定矿石种类与分布，并填充 `_minerals`。
  - 可视化：`MineralSystem::attachTo` 绑定世界节点后，通过 `syncVisuals` 懒创建 `Game::Mineral` 节点并挂到场景中，保持“系统持有状态 + 节点由系统统一创建/销毁”的约束。
- 交互入口：
  - 采矿：镐子等工具通过地图控制器 `MineMapController::obstacleSystem(ObstacleKind::Mineral)` 获取矿物系统接口，在命中矿石时调用系统的受击接口降低 HP；当 HP 降为 0 时，由系统调用 `mineralDropItem` 生成掉落并通过回调交给 `DropSystem`。
  - 楼梯遮挡：`StairSystem` 在生成楼梯时会参考 `MineralSystem` 的矿石分布，避免楼梯被巨岩等矿石完全覆盖。

**关键类示例（Mineral）**

- `MineralSystem`：矿石/石块状态与节点唯一来源，负责生成/刷新/受击与销毁。
- `Game::Mineral` / `Game::MineralData` / `Game::MineralType`：矿物实体节点与运行时数据结构。
- `Game::mineralDropItem`：矿石类型到掉落物品的映射函数。
- `MineMapController`：为矿物系统提供楼层信息、坐标换算与障碍查询。

### 可放置物（Placeable Item）

- 几何与通用规则：`PlaceableItemBase`（`Classes/Game/PlaceableItem/PlaceableItemBase.*`）抽象出所有可放置物体（箱子/熔炉等）的公共几何接口：`pos`、`placeRect()`、`collisionRect()`、`maxPerArea()`，并提供 `isNearAny`、`canPlaceAt` 等模板工具，用于统一实现“接近判定”和“可放置判定”。
- 系统基类：`PlaceableItemSystemBase`（`Classes/Controllers/Systems/PlaceableItemSystemBase.*`）负责管理可放置物系统的 `DrawNode`，并提供统一的 `InteractWithItem` 入口：先判断是否应进入“放置流程”，否则尝试与已有物体交互；具体规则由子类实现。
- 状态唯一来源：
  - 箱子：`ChestController`（`Classes/Controllers/Systems/ChestController.*`）作为箱子状态的唯一来源，持有当前地图所有箱子的列表，并与 `WorldState::farmChests` / `houseChests` 等容器同步。
  - 熔炉：`FurnaceController`（`Classes/Controllers/Systems/FurnaceController.*`）统一管理熔炉列表与计时逻辑，底层数据存放在 `WorldState` 的各个 `*Furnaces` 容器。
- 交互入口：
  - 放置：`PlacementInteractor`（`Classes/Controllers/Interact/PlacementInteractor.*`）根据地图类型与玩家位置，选择合适的放置中心点，再调用对应系统的 `tryPlace` 完成放置与背包扣除。
  - 箱子交互：右键或交互键触发 `ChestController::tryInteractExisting`，打开 `ChestPanelUI`（`Classes/Controllers/UI/ChestPanelUI.*`）进行物品转移；业务规则（容量/堆叠/移动）集中在 `ChestController` 与背包系统内部。
  - 熔炉交互：熔炉系统在交互时根据 `Game::furnaceRecipeFor` 查找配方，将矿石与燃料转化为锭，完成后通过掉落或直接写回背包。

**关键类示例（Placeable Item）**

- `PlaceableItemBase`：可放置物基础几何接口与放置判定工具。
- `PlaceableItemSystemBase`：所有可放置物系统的统一调度基类。
- `ChestController` / `FurnaceController`：箱子/熔炉系统的具体实现与状态唯一来源。
- `Game::Chest` / `Game::Furnace`：可放置物体的运行时数据结构。

### 工具升级（Tool Upgrade）

- 规则唯一来源：`ToolUpgradeSystem`（`Classes/Controllers/Systems/ToolUpgradeSystem.*`）以单例形式存在，通过 `getInstance()` 获取；集中负责工具等级查询、下一次升级消耗计算与实际升级操作，是“工具升级规则”的唯一来源。
- 与工具数据的关系：工具基类 `Game::ToolBase`（`Classes/Game/Tool/ToolBase.*`）持有 `level` 字段，具体工具（锄头/镐子/斧头/浇水壶等）在自身逻辑中根据等级调整伤害、效率或范围；`ToolUpgradeSystem` 通过背包与全局箱子中的 `ToolBase` 实例读写该等级。
- 升级规则：
  - 等级上限：当前约定等级 0~3 共四档，`nextUpgradeCost` 在等级已达上限或未找到工具时返回 `false`。
  - 消耗结构：从 0 升 1 需要 2000G + 5 个铜锭，从 1 升 2 需要 5000G + 5 个铁锭，从 2 升 3 需要 10000G + 5 个金锭；材料类型分别为 `ItemType::CopperIngot` / `IronIngot` / `GoldIngot`。
  - 统一扣费：`upgradeToolOnce` 在确认可升级后从 `WorldState` 扣除金币，并从玩家背包（必要时扩展到全局箱子）中扣除对应材料，再将工具等级 +1。
- UI 与入口：
  - 面板：`ToolUpgradePanelUI`（`Classes/Controllers/UI/ToolUpgradePanelUI.*`）负责构建“工具升级” UI（图标/等级文本/材料图标/按钮），在 `refreshPanel` 中调用 `ToolUpgradeSystem::nextUpgradeCost` 获取升级信息并更新按钮状态。
  - 触发：`UIController` 提供 `buildToolUpgradePanel` / `toggleToolUpgradePanel` 等方法，由场景或输入逻辑在合适时机打开/关闭升级面板；面板内部按钮点击通过 `ToolUpgradeSystem::upgradeToolOnce` 执行实际升级，并在成功后回调 `UIController::refreshHotbar` 刷新热键栏显示。

**关键类示例（Tool Upgrade）**

- `ToolUpgradeSystem`：工具升级规则与执行入口的唯一来源。
- `Game::ToolBase` 及其派生工具：持有等级并在 `use` 逻辑中根据等级调整效果。
- `ToolUpgradePanelUI`：工具升级 UI 面板，展示等级/材料并驱动系统调用。
- `UIController`：作为升级面板的聚合入口与升级后 UI 刷新触发点。

### 配方与合成（Recipe / Craft）

- 数据抽象：`RecipeBase`（`Classes/Game/Recipe/RecipeBase.*`）定义“材料列表 → 产物”的统一接口，提供可合成判定与执行逻辑。
- 配方集中管理：`RecipeBook`（`Classes/Game/Recipe/RecipeBook.*`）集中创建并持有所有配方实例，避免配方散落在多个模块维护；UI 通过 `RecipeFilter` 做分类筛选。
- 合成控制器：`CraftingController`（`Classes/Controllers/Crafting/CraftingController.*`）只负责扣材料与加产物，不负责 UI 与输入。
- UI 入口：`CraftPanelUI`（`Classes/Controllers/UI/CraftPanelUI.*`）负责列表、分页与筛选展示，并把点击事件转发给 `CraftingController` 执行合成。

**关键类示例（Recipe / Craft）**

- `RecipeBase` / `SimpleRecipe`：配方接口与数据驱动实现。
- `RecipeBook` / `CategoryRecipeFilter`：配方表与筛选器。
- `CraftingController`：合成执行入口（背包增删的唯一落点）。
- `CraftPanelUI`：合成面板 UI（列表展示 + 触发合成）。

### 商店（Store）

- 交易规则：`StoreController`（`Classes/Controllers/Store/StoreController.*`）负责买入/卖出时的金币、精力与背包物品变更，UI 只通过接口触发交易。
- 商品展示：`StorePanelUI`（`Classes/Controllers/UI/StorePanelUI.*`）提供分类、分页与买卖入口；内部持有 `StoreController` 完成交易闭环。
- 触发入口：NPC 交互接口 `NpcControllerBase`（`Classes/Controllers/NPC/NpcControllerBase.*`）在交互回调中调用 `UIController` 打开/关闭商店面板，避免把“开店逻辑”写进场景类。
- 动物商店：`AnimalStorePanelUI` 只负责购买界面展示，真实扣费/生成由上层回调委托给 `AnimalSystem`。

**关键类示例（Store）**

- `StoreController`：交易规则与金币/背包变更入口。
- `StorePanelUI`：杂货商店 UI（分类/分页/买卖）。
- `NpcControllerBase`：商店触发接口位置（对话/右键等交互回调）。
- `AnimalStorePanelUI`：动物商店 UI（回调驱动购买）。

### 技能树（SkillTree）

- 静态定义：`SkillTreeBase`（`Classes/Game/SkillTree/SkillTreeBase.*`）描述技能树类型、节点列表与经验曲线；各 `*SkillTree.cpp` 只提供常量节点定义。
- 进度与加成入口：`SkillTreeSystem`（`Classes/Game/SkillTree/SkillTreeSystem.*`）负责读写 `WorldState` 中的技能进度，提供“加经验/查等级/查加成描述”等统一接口。
- 经验来源：作物收获与动物喂食等系统在规则落点处调用 `SkillTreeSystem::addXp`，保证经验累积与业务规则同源。
- UI 展示：`SkillTreePanelUI`（`Classes/Controllers/UI/SkillTreePanelUI.*`）只负责展示与刷新，数据查询统一走 `SkillTreeSystem`。

**关键类示例（SkillTree）**

- `SkillTreeBase` / `SkillNode` / `SkillProgress`：静态定义与进度数据结构。
- `SkillTreeSystem`：技能经验、等级、点数与加成的统一入口。
- `SkillTreePanelUI`：技能面板 UI（展示等级/经验/加成）。

### 工具与环境交互链路

围绕“玩家工具使用”这一高频操作，项目中形成了一条相对统一的交互链路：

- 输入采集：`PlayerController` 根据键盘/鼠标输入与当前背包选中物品，判断是否触发某个 Tool 的使用。
- 工具逻辑：Tool 本身只做“调用对应 System 接口”，例如锄头作用于 `CropSystem`、武器/镐子作用于矿洞/环境系统（如 `MineralSystem`）、斧子作用于 `TreeSystem` 等。
- 系统判定：各环境 System 内部完成命中判定、耐久/HP 计算、破坏与否、掉落内容等业务逻辑，并返回结构化结果（如是否命中、是否摧毁、掉落列表）。
- 结果分发：Controller 根据系统返回的结果，调用 `DropSystem` 生成掉落、更新 `WorldState`，并通知 `UIController` 刷新背包/提示，同时可能驱动 `AudioManager` 播放音效、摄像机震动等效果。

通过上述分层，项目中同一类职责尽量集中在同一类/同一模块中，避免出现“同一实体状态在多处维护”或“场景直接堆业务逻辑”的情况，也便于后续对单个模块进行替换与扩展。

## 开发流程（基于提交记录归纳）

### （1）初期阶段

- 搭建基础运行骨架：完成 `SceneBase` 的通用初始化、地图控制器抽象、UI 与背包的基础联动。
- 打通核心数据链路：以 `WorldState` 作为跨场景共享与持久化的数据中心，并以 `SaveSystem` 支持多槽位存档。

### （2）迭代阶段

- 玩法快速扩展：围绕“工具 -> 系统 -> 掉落/经验/UI 刷新”闭环逐步接入钓鱼、合成、矿洞、电梯、工具升级等模块。
- 频繁修复与体验打磨：例如热键栏显示、箱子取物重复、地图卡住点、怪物掉落等问题在迭代中持续被修复。

### （3）后期阶段

- 规范化与收敛：统一文件编码与行尾（提交信息“统一 UTF-8/CRLF”），补齐头文件中文注释与模块职责说明，整理目录与命名勘误。
- 文档与演示材料完善：补充项目文档与演示 PPT，形成可交付材料。

## 开发中遇到的问题及解决方法

- 钓鱼节与天气逻辑冲突：天气使用固定 seed 计算导致“夏季第 6 天”可能必然下雨，影响节日体验；解决方式是在节日当天强制 `ws.isRaining = false`。
- 钓鱼小游戏难度不合理：鱼速度/加速度导致难以捕获；通过限制速度上限并调整阻尼与进度增减速率改善手感。
- cocos2dx使用的OpenGL坐标与瓦片坐标的冲突：OpenGL坐标y轴朝上，而游戏内使用的瓦片坐标为y轴朝下。这个问题导致使用工具前，获取鼠标点击坐标时，出现了上下相反的问题。一开始并未意识到是由坐标系冲突导致，而是简单地将点击的坐标上下镜像尝试解决，而导致了人物在接近地图边界，离开屏幕中心时的定位问题。后续通过仔细排查问题发生的链路，找到根源问题，并将鼠标点击的OpenGL坐标值正确换算为瓦片坐标值，解决了问题。
- 箱子/房间切换显示异常：迭代中出现“切回 Room 场景箱子贴图消失”等问题，通过重命名与交互器职责调整修复。

## 开发项目过程学习到的知识

- 游戏项目的“数据唯一来源”设计：用 `WorldState` 集中管理持久化数据，减少跨场景状态分叉，降低 bug 产生概率。
- 以接口隔离地图差异：通过 `IMapController` 把 TMX/碰撞/坐标系统差异封装在地图层，玩法系统只依赖抽象接口。
- 事件驱动与解耦：用 lambda 回调把“工具行为/系统调用/UI 刷新”串起来，减少模块间的直接依赖。
- 存档兼容与演进：以版本号控制增量字段读取，并在读取过程中做边界与流状态检查，提升旧存档兼容性。
