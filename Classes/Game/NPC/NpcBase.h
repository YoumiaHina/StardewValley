#pragma once

#include "Game/Item.h"

namespace Game {

// NPC 逻辑基类：
// - 提供名称、纹理路径与赠礼好感计算的统一接口。
// - 具体 NPC（Abigail/Pierre/Robin/Willy 等）只关心各自的配置与规则。
class NpcBase {
 public:
  virtual ~NpcBase() = default;
  // NPC 名称：用于 UI 显示与对话框标题。
  virtual const char* name() const = 0;
  // NPC 默认站立姿势的纹理路径，用于生成精灵。
  virtual const char* texturePath() const = 0;
  // 玩家赠送物品时的好感增量，按物品类型返回分数。
  virtual int friendshipGainForGift(ItemType itemType) const = 0;
};

}  // namespace Game
