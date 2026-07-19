#pragma once
#include <random>
#include <algorithm>
#include "Enemy.hpp"
#include "EntityPool.hpp"

// Owns the spawn timer and difficulty curve, and hands off newly
// spawned enemies into the EntityPool.
class EnemySpawner {
public:
    EnemySpawner(EntityPool<Enemy>& poolRef, sf::Vector2u windowSize)
        : pool(poolRef), winSize(windowSize), rng(std::random_device{}())
    {}

    void update(float dt, float elapsedTime, int playerLevel, const Player& player) {
        spawnTimer += dt;

        float interval = spawnInterval(elapsedTime);
        if (spawnTimer >= interval) {
            spawnTimer = 0.f;
            spawnOne(elapsedTime, playerLevel, player);
        }
    }

private:
    // Tightened from the original design-doc draft: starts faster (1.5s
    // vs 2.0s) and ramps down quicker (0.015/sec vs 0.01/sec), so the
    // early game isn't quite so sparse.
    float spawnInterval(float elapsedTime) const {
        return std::max(0.25f, 1.5f - 0.015f * elapsedTime);
    }

    // Combines time survived AND player level, so a player who's
    // leveling up fast (killing lots of enemies) faces rising
    // difficulty too, not just whoever waits around longest.
    float difficultyMultiplier(float elapsedTime, int playerLevel) const {
        float timeFactor = 1.f + 0.02f * (elapsedTime / 60.f);   // +2% per minute survived
        float levelFactor = 1.f + 0.08f * (playerLevel - 1);      // +8% per player level
        return timeFactor * levelFactor;
    }

    EnemyType pickType(float elapsedTime, int playerLevel) {
        std::uniform_real_distribution<float> roll(0.f, 1.f);
        float r = roll(rng);

        // Variety unlocks earlier now, and level also opens up tougher
        // types sooner for a player who's leveling quickly.
        bool runnersUnlocked = elapsedTime > 20.f || playerLevel >= 3;
        bool brutesUnlocked = elapsedTime > 90.f || playerLevel >= 6;

        if (brutesUnlocked && r < 0.2f) return EnemyType::Brute;
        if (runnersUnlocked && r < 0.5f) return EnemyType::Runner;
        return EnemyType::Grunt;
    }

    sf::Vector2f pickEdgePosition() {
        std::uniform_int_distribution<int> sideDist(0, 3);
        std::uniform_real_distribution<float> xDist(0.f, static_cast<float>(winSize.x));
        std::uniform_real_distribution<float> yDist(0.f, static_cast<float>(winSize.y));

        switch (sideDist(rng)) {
            case 0: return {xDist(rng), -30.f};
            case 1: return {xDist(rng), winSize.y + 30.f};
            case 2: return {-30.f, yDist(rng)};
            default: return {winSize.x + 30.f, yDist(rng)};
        }
    }

    void spawnOne(float elapsedTime, int playerLevel, const Player& player) {
        Enemy* enemy = pool.spawn();
        if (!enemy) return; // pool exhausted, skip this spawn silently

        EnemyType type = pickType(elapsedTime, playerLevel);
        sf::Vector2f pos = pickEdgePosition();
        float multiplier = difficultyMultiplier(elapsedTime, playerLevel);
        enemy->reset(type, pos, &player, multiplier);
    }

    EntityPool<Enemy>& pool;
    sf::Vector2u winSize;
    std::mt19937 rng;
    float spawnTimer = 0.f;
};
