# StardewValley（Cocos2d-x / C++）

本仓库是一个基于 **Cocos2d-x 3.17.2** 的 2D 模拟经营/冒险类小游戏项目（类《星露谷物语》玩法拆解），核心代码位于 `Classes/`，资源位于 `Resources/`，当前主要面向 **Windows（Win32）** 构建与运行。

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
- 商店：`Classes/Controllers/Store/StoreController.*` + `Classes/Controllers/UI/StorePanelUI.*`
- NPC / 社交 / 对话：`Classes/Controllers/NPC/*` + `Classes/Controllers/UI/NpcSocialPanelUI.*` + `Classes/Controllers/UI/DialogueUI.*`
- 合成/配方：`Classes/Controllers/Crafting/CraftingController.*` + `Classes/Game/Recipe/*`
- 熔炉：`Classes/Controllers/Systems/FurnaceController.*`
- 工具升级：`Classes/Controllers/Systems/ToolUpgradeSystem.*`
- 掉落：`Classes/Controllers/Systems/DropSystem.*`
- 音乐/音频：`Classes/Controllers/Managers/AudioManager.*`
- 采矿/战斗/怪物（矿洞）：`Classes/Controllers/Mine/*` + `Classes/Game/Monster/*`
- 环境障碍物（树/石头/矿石/杂草/楼梯等）：`Classes/Controllers/Environment/*`
- UI 聚合入口：`Classes/Controllers/UI/UIController.*`（统一管理 HUD/背包/面板等）

## 玩法系统简述

### 社交系统（NPC / 对话 / 好感度）

- NPC 分布（按场景组装）：城镇 `Abigail` / `Pierre`，农场 `Robin`，海滩 `Willy`。
- 对话系统：`NpcDialogueManager` 将 NPC 对话树映射为 UI 文本/选项，并通过 `UIController::showDialogue` 弹出 `DialogueUI`。
- 对话操作：有选项时点击按钮选择；无选项时点击对话框区域推进。
- 好感度：保存在 `Game::WorldState::npcFriendship`，部分 NPC 支持“赠礼提升好感（每日一次）”，超出会提示 `Already gifted today`。
- 社交面板：鼠标右键点击 NPC 精灵可打开社交面板，展示名称、好感、关系状态与任务列表。
- 任务/恋爱：`WorldState` 预留 `npcQuests` / `npcRomanceUnlocked` 存档字段并支持在社交面板展示；任务生成/恋爱解锁逻辑目前未在代码中实现入口（仅存档与 UI 读取）。
- 节日（Festival）：`SceneBase::initBase` 统一创建 `FestivalController` 并挂到 update 回调；当“夏季第 `GameConfig::FESTIVAL_DAY` 天（当前为 6）”到来时，控制器会通过 `IMapController::setFestivalActive(bool)` 同步节日开关到当前地图。
- 节日地图表现：小镇/海滩会显示/隐藏 TMX 的 `Festival` 图层；小镇在节日启用时额外解析 `FestivalWall` 作为碰撞来源。

### 钓鱼系统（Fishing）

- 创建与更新：`SceneBase::initBase` 统一创建 `FishingController` 并每帧调用 `update`。
- 工具入口：背包默认包含 `FishingRod`，并在初始化时把钓鱼竿的启动回调绑定到 `FishingController::startAt`。
- 交互提示：当玩家“靠近湖边 + 当前选中钓鱼竿 + 未处于钓鱼中”时，UI 会提示 `Left-click to Fish`。
- 小游戏规则：`startAt` 会校验冷却与“湖边可钓区域”，开始后锁定移动并置 `WorldState::fishingActive=true`；按住/松开鼠标左键控制绿色捕获条上下，重叠越多进度越快，进度满则成功发放鱼类与钓鱼经验，时间耗尽则失败。
- 节日钓鱼联动：在节日当天（同 `GameConfig::FESTIVAL_DAY`）会从 `kFestivalFish` 表随机一条“节日鱼”作为外观与最终奖励。

### 商店系统

- 杂货店：城镇的 `Pierre` 通过对话选项或右键直接打开商店面板。
- 动物商店：农场的 `Robin` 打开动物商店面板，购买行为由场景侧回调驱动，并最终由 `AnimalSystem` 生成动物。

### 农场管理（作物 / Crop）

- 状态唯一来源：`Controllers::CropSystem` 维护运行时作物列表，并与 `Game::WorldState::farmCrops` 同步持久化。
- 播种（种子 -> 作物类型）：交互发生在 `FarmInteractor`（空格/左键）里，满足“目标格可耕 + 无作物占用 + 手持种子 + 季节允许”后调用 `CropSystem::plantCrop` 并消耗背包物品。
- 浇水：`WaterCan` 在瓦片为 `Tilled` 时将其置为 `Watered`，并调用 `CropSystem::markWateredAt` 写入“当日已浇水”标记
- 收获（锄头）：`Hoe` 命中作物格子时走 `CropSystem::harvestByHoeAt`，由系统计算产物/数量并在成功后执行移除或回生状态转换；若产物溢出背包则落地为掉落物。
- 生长与日结：睡觉到第二天会触发 `CropSystem::advanceCropsDaily`，按“浇水推进/缺水概率枯死/季节不符枯死”更新作物，并把 `Watered` 瓦片回退为 `Tilled`。
- 可视化刷新：农场地图用 `FarmMapController::refreshCropsVisuals` 读取 `WorldState::farmCrops` 并生成/更新作物精灵；交互（播种/收获）后会主动调用该刷新。

### 农场管理（动物 / Animal）

- 状态唯一来源：`Controllers::AnimalSystem` 维护运行时动物实例列表（含精灵/标签节点），并持续写回 `Game::WorldState::farmAnimals`；每日推进逻辑读取 `farmAnimals` 并产出掉落。
- 购买入口：`RobinNpcController` 通过空格对话选项或右键直接打开动物商店面板，面板渲染与“Buy”点击回调在 `AnimalStorePanelUI`。
- 购买落地：`FarmScene` 将动物商店的购买回调绑定为“在玩家前方一格生成动物”，并调用 `AnimalSystem::buyAnimal`；价格来自 `Game::animalPrice`。
- 喂食：`FarmInteractor` 在空格交互时优先尝试喂食（距离阈值/可接受饲料/当日未喂食），成功后由上层扣除背包物品并刷新 UI。
- 每日产物与掉落：睡觉到第二天会调用 `advanceAnimalsDaily` 推进成长与产物；若当前在农场则直接生成地图掉落，否则写入 `WorldState::farmDrops` 等待回到农场再落地。

### 资源采集与环境障碍物

- 树/石头/矿石/杂草/楼梯等统一由对应 `*System` 维护状态与可视化挂载。
- 场景与交互器只负责输入转发与调用顺序，不应在 Controller/Scene 内维护重复的环境实体容器。

### 矿洞（战斗 / 楼层 / 电梯）

- 矿洞场景入口与楼层切换：`Classes/Scenes/MineScene.cpp`
- 怪物与战斗：`Classes/Controllers/Mine/*`
- 电梯：在特定楼层解锁，并通过电梯面板选择回到已解锁楼层。

### 时间 / 天气 / 日结

- 时间推进与每日结算（作物生长等）：`Classes/Controllers/Systems/GameStateController.*`
- 天气表现：`Classes/Controllers/Systems/WeatherController.*`，并通过 `WorldState` 持久化当天是否下雨等标记。

### 音乐系统（BGM）

- 管理器：`AudioManager` 作为单例封装 `AudioEngine`，保证同一时刻只播放一首 BGM，并避免重复切歌。
- 场景切换：各场景在 `init()` 时选择 `SceneZone` 并调用 `playBackgroundFor`（房间 `Classes/Scenes/RoomScene.cpp:24-29`，农场 `Classes/Scenes/FarmScene.cpp:29-34`，矿洞 `Classes/Scenes/MineScene.cpp:13-19`）。
- 资源映射：`SceneZone -> 音乐路径` 由 `AudioManager::pathForZone` 决定，对应资源位于 `Resources/music/`。

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
  - `Ctrl`：切换工具（工具需升至满级）距离修饰（用于扩大交互范围的按住状态）

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

- C++11/14 特性：大量使用 lambda 回调做模块解耦与事件绑定（例如钓鱼竿启动回调绑定）；单例采用函数内 `static`（线程安全）初始化。
- 架构清晰：SceneBase 只承担“骨架 + 调度 + 转发”，业务逻辑下沉到 `Controllers/*` 与 `Game/*`，以接口对接减少跨模块耦合。
- 目录结构明确：`Classes/Scenes` 放场景骨架；`Classes/Controllers` 放控制器/系统；`Classes/Game` 放数据结构、道具与规则定义；`Resources` 放地图/贴图/音乐。

### （4）界面与体验

- UI 聚合：`UIController` 作为统一入口管理 HUD/热键栏/背包与各类面板，避免 UI 分散在多个系统中互相直接引用。
- 交互一致：社交/对话/钓鱼/箱子/商店等面板打开时统一阻止移动，减少输入冲突。

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
- NPC 赠礼重复结算：需要“每日一次赠礼”约束；通过 `npcLastGiftDay` 记录当天赠礼时间并在重复赠礼时提示。
- 箱子/房间切换显示异常：迭代中出现“切回 Room 场景箱子贴图消失”等问题，通过重命名与交互器职责调整修复。

## 开发项目过程学习到的知识

- 游戏项目的“数据唯一来源”设计：用 `WorldState` 集中管理持久化数据，减少跨场景状态分叉，降低 bug 产生概率。
- 以接口隔离地图差异：通过 `IMapController` 把 TMX/碰撞/坐标系统差异封装在地图层，玩法系统只依赖抽象接口。
- 事件驱动与解耦：用 lambda 回调把“工具行为/系统调用/UI 刷新”串起来，减少模块间的直接依赖。
- 存档兼容与演进：以版本号控制增量字段读取，并在读取过程中做边界与流状态检查，提升旧存档兼容性。
