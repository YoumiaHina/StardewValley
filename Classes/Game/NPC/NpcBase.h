#pragma once

#include "Game/Item.h"

namespace Game {

class NpcBase {
 public:
  virtual ~NpcBase() = default;
  virtual const char* name() const = 0;
  virtual const char* texturePath() const = 0;
  virtual int friendshipGainForGift(ItemType itemType) const = 0;
};

}  // namespace Game
