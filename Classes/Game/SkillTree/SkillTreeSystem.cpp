#include "Game/SkillTree/SkillTreeSystem.h"

#include <algorithm>
#include <limits>
#include <random>

namespace Game {

SkillTreeSystem& SkillTreeSystem::getInstance() {
    static SkillTreeSystem inst;
    return inst;
}

const SkillTreeBase& SkillTreeSystem::definition(SkillTreeType type) const {
    return skillTreeByType(type);
}

std::size_t SkillTreeSystem::indexFromType(SkillTreeType type) const {
    switch (type) {
        case SkillTreeType::Farming: return 0;
        case SkillTreeType::AnimalHusbandry: return 1;
        case SkillTreeType::Forestry: return 2;
        case SkillTreeType::Fishing: return 3;
    }
    return 0;
}

void SkillTreeSystem::normalizeProgress(SkillTreeType type) const {
    auto& ws = globalState();
    std::size_t idx = indexFromType(type);
    if (idx >= ws.skillTrees.size()) return;

    auto& p = ws.skillTrees[idx];
    if (p.totalXp < 0) p.totalXp = 0;
    if (p.unspentPoints < 0) p.unspentPoints = 0;

    auto& def = definition(type);
    int lvl = def.levelFromTotalXp(p.totalXp);
    int earnedPoints = std::max(0, lvl * def.pointsPerLevel());

    std::sort(p.unlockedNodeIds.begin(), p.unlockedNodeIds.end());
    p.unlockedNodeIds.erase(std::unique(p.unlockedNodeIds.begin(), p.unlockedNodeIds.end()), p.unlockedNodeIds.end());

    int spentPoints = static_cast<int>(p.unlockedNodeIds.size());
    int expectedUnspent = std::max(0, earnedPoints - spentPoints);
    p.unspentPoints = expectedUnspent;
}

int SkillTreeSystem::level(SkillTreeType type) const {
    normalizeProgress(type);
    auto& ws = globalState();
    const auto& p = ws.skillTrees[indexFromType(type)];
    return definition(type).levelFromTotalXp(p.totalXp);
}

int SkillTreeSystem::totalXp(SkillTreeType type) const {
    normalizeProgress(type);
    auto& ws = globalState();
    return ws.skillTrees[indexFromType(type)].totalXp;
}

int SkillTreeSystem::xpToNextLevel(SkillTreeType type) const {
    normalizeProgress(type);
    auto& ws = globalState();
    const auto& p = ws.skillTrees[indexFromType(type)];
    SkillProgress tmp;
    tmp.totalXp = p.totalXp;
    tmp.level = definition(type).levelFromTotalXp(p.totalXp);
    return definition(type).xpToNextLevel(tmp);
}

int SkillTreeSystem::unspentPoints(SkillTreeType type) const {
    normalizeProgress(type);
    auto& ws = globalState();
    return ws.skillTrees[indexFromType(type)].unspentPoints;
}

bool SkillTreeSystem::isNodeUnlocked(SkillTreeType type, int nodeId) const {
    normalizeProgress(type);
    auto& ws = globalState();
    const auto& p = ws.skillTrees[indexFromType(type)];
    return std::binary_search(p.unlockedNodeIds.begin(), p.unlockedNodeIds.end(), nodeId);
}

bool SkillTreeSystem::unlockNode(SkillTreeType type, int nodeId) {
    normalizeProgress(type);
    auto& ws = globalState();
    auto& p = ws.skillTrees[indexFromType(type)];
    const auto& def = definition(type);

    int lvl = def.levelFromTotalXp(p.totalXp);
    if (p.unspentPoints <= 0) return false;
    if (std::binary_search(p.unlockedNodeIds.begin(), p.unlockedNodeIds.end(), nodeId)) return false;
    const SkillNode* node = def.findNode(nodeId);
    if (!node) return false;
    if (lvl < node->requiredLevel) return false;

    p.unlockedNodeIds.push_back(nodeId);
    normalizeProgress(type);
    return true;
}

bool SkillTreeSystem::unlockFirstAvailableNode(SkillTreeType type, int* outUnlockedNodeId) {
    if (outUnlockedNodeId) *outUnlockedNodeId = 0;
    normalizeProgress(type);
    auto& ws = globalState();
    auto& p = ws.skillTrees[indexFromType(type)];
    const auto& def = definition(type);

    int lvl = def.levelFromTotalXp(p.totalXp);
    if (p.unspentPoints <= 0) return false;

    for (const auto& node : def.nodes()) {
        if (lvl < node.requiredLevel) continue;
        if (std::binary_search(p.unlockedNodeIds.begin(), p.unlockedNodeIds.end(), node.id)) continue;
        if (unlockNode(type, node.id)) {
            if (outUnlockedNodeId) *outUnlockedNodeId = node.id;
            return true;
        }
    }
    return false;
}

void SkillTreeSystem::addXp(SkillTreeType type, int deltaXp, int* outLevelsGained) {
    if (deltaXp <= 0) {
        if (outLevelsGained) *outLevelsGained = 0;
        return;
    }
    normalizeProgress(type);
    auto& ws = globalState();
    auto& p = ws.skillTrees[indexFromType(type)];

    const auto& def = definition(type);
    int beforeLevel = def.levelFromTotalXp(p.totalXp);
    long long next = static_cast<long long>(p.totalXp) + static_cast<long long>(deltaXp);
    if (next < 0) next = 0;
    if (next > static_cast<long long>(std::numeric_limits<int>::max())) {
        next = std::numeric_limits<int>::max();
    }
    p.totalXp = static_cast<int>(next);
    normalizeProgress(type);

    int afterLevel = def.levelFromTotalXp(p.totalXp);
    if (outLevelsGained) *outLevelsGained = std::max(0, afterLevel - beforeLevel);
}

float SkillTreeSystem::farmingExtraProduceChance() const {
    int lvl = level(SkillTreeType::Farming);
    float chance = std::min(0.25f, 0.02f * static_cast<float>(lvl));
    if (isNodeUnlocked(SkillTreeType::Farming, 102)) {
        chance += 0.10f;
    }
    if (chance < 0.0f) chance = 0.0f;
    if (chance > 0.60f) chance = 0.60f;
    return chance;
}

float SkillTreeSystem::forestryExtraWoodChance() const {
    int lvl = level(SkillTreeType::Forestry);
    float chance = std::min(0.20f, 0.015f * static_cast<float>(lvl));
    if (isNodeUnlocked(SkillTreeType::Forestry, 301)) {
        chance += 0.12f;
    }
    if (chance < 0.0f) chance = 0.0f;
    if (chance > 0.60f) chance = 0.60f;
    return chance;
}

float SkillTreeSystem::fishingExtraFishChance() const {
    int lvl = level(SkillTreeType::Fishing);
    float chance = std::min(0.20f, 0.015f * static_cast<float>(lvl));
    if (isNodeUnlocked(SkillTreeType::Fishing, 402)) {
        chance += 0.12f;
    }
    if (chance < 0.0f) chance = 0.0f;
    if (chance > 0.60f) chance = 0.60f;
    return chance;
}

float SkillTreeSystem::husbandryExtraProductChance() const {
    int lvl = level(SkillTreeType::AnimalHusbandry);
    float chance = std::min(0.18f, 0.012f * static_cast<float>(lvl));
    if (isNodeUnlocked(SkillTreeType::AnimalHusbandry, 202)) {
        chance += 0.15f;
    }
    if (chance < 0.0f) chance = 0.0f;
    if (chance > 0.60f) chance = 0.60f;
    return chance;
}

std::string SkillTreeSystem::bonusDescription(SkillTreeType type) const {
    if (type == SkillTreeType::Farming) {
        float c = farmingExtraProduceChance();
        int pct = static_cast<int>(c * 100.0f + 0.5f);
        return std::string("收获额外+1概率 ") + std::to_string(pct) + "%";
    }
    if (type == SkillTreeType::Forestry) {
        float c = forestryExtraWoodChance();
        int pct = static_cast<int>(c * 100.0f + 0.5f);
        return std::string("木材额外+1概率 ") + std::to_string(pct) + "%";
    }
    if (type == SkillTreeType::Fishing) {
        float c = fishingExtraFishChance();
        int pct = static_cast<int>(c * 100.0f + 0.5f);
        return std::string("额外+1鱼概率 ") + std::to_string(pct) + "%";
    }
    if (type == SkillTreeType::AnimalHusbandry) {
        float c = husbandryExtraProductChance();
        int pct = static_cast<int>(c * 100.0f + 0.5f);
        return std::string("产物额外+1概率 ") + std::to_string(pct) + "%";
    }
    return std::string("暂无加成");
}

int SkillTreeSystem::adjustHarvestQuantityForFarming(Game::ItemType, int baseQty) const {
    if (baseQty <= 0) return baseQty;
    float chance = farmingExtraProduceChance();
    if (chance <= 0.0001f) return baseQty;

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(rng);
    if (roll < chance) {
        return baseQty + 1;
    }
    return baseQty;
}

int SkillTreeSystem::xpForFarmingHarvest(Game::ItemType, int harvestedQty) const {
    int qty = std::max(0, harvestedQty);
    return 10 + qty * 1;
}

int SkillTreeSystem::adjustWoodDropQuantityForForestry(int baseQty) const {
    if (baseQty <= 0) return baseQty;
    float chance = forestryExtraWoodChance();
    if (chance <= 0.0001f) return baseQty;

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(rng);
    if (roll < chance) {
        return baseQty + 1;
    }
    return baseQty;
}

int SkillTreeSystem::xpForForestryChop(int woodQty) const {
    int qty = std::max(0, woodQty);
    return 8 + qty * 1;
}

int SkillTreeSystem::adjustFishCatchQuantityForFishing(int baseQty) const {
    if (baseQty <= 0) return baseQty;
    float chance = fishingExtraFishChance();
    if (chance <= 0.0001f) return baseQty;

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(rng);
    if (roll < chance) {
        return baseQty + 1;
    }
    return baseQty;
}

int SkillTreeSystem::xpForFishingCatch(int fishQty) const {
    int qty = std::max(0, fishQty);
    return 12 + qty * 2;
}

int SkillTreeSystem::adjustAnimalProductQuantityForHusbandry(Game::ItemType, int baseQty) const {
    if (baseQty <= 0) return baseQty;
    float chance = husbandryExtraProductChance();
    if (chance <= 0.0001f) return baseQty;

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float roll = dist(rng);
    if (roll < chance) {
        return baseQty + 1;
    }
    return baseQty;
}

int SkillTreeSystem::xpForAnimalProduct(Game::ItemType, int qty) const {
    int n = std::max(0, qty);
    return 10 + n * 2;
}

}
