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

class Abigail : public NpcBase {
 public:
  const char* name() const override;
  const char* texturePath() const override;
  int friendshipGainForGift(ItemType itemType) const override;
};

class Willy : public NpcBase {
 public:
  const char* name() const override;
  const char* texturePath() const override;
  int friendshipGainForGift(ItemType itemType) const override;
};

}  // namespace Game

