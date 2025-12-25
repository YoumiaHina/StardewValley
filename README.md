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

### 矿洞冒险（战斗 / 楼层 / 电梯）

- 从入口进入矿洞，逐层深入探索，每层都有怪物与资源点。
- 使用武器与怪物战斗，击败怪物可获得金币与掉落。
- 电梯会随着探索逐步解锁，允许快速前往已解锁楼层，减少重复跑图。
- 在矿洞中需要关注生命值与精力，适时返回农场补给与存档。

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

- C++11/14 特性：合理使用 lambda 回调做模块解耦与事件绑定（例如钓鱼竿启动回调绑定）；单例采用函数内 `static`（线程安全）初始化。
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
