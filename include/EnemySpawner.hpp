#pragma once
#include <random>
#include <algorithm>
#include "Enemy.hpp"
#include "EntityPool.hpp"

// Owns the spawn timer and difficulty curve, and hands off newly
// spawned enemies into the EntityPool. Matches the formulas already
// written in docs/design-doc.md.
class EnemySpawner {
public:
    EnemySpawner(EntityPool<Enemy>& poolRef, sf::Vector2u windowSize)
        : pool(poolRef), winSize(windowSize), rng(std::random_device{}())
    {}

    void update(float dt, float elapsedTime, const Player& player) {
        spawnTimer += dt;

        float interval = spawnInterval(elapsedTime);
        if (spawnTimer >= interval) {
            spawnTimer = 0.f;
            spawnOne(elapsedTime, player);
        }
    }

private:
    // Matches: spawn_interval(t) = max(0.3, 2.0 - 0.01 * t_seconds)
    float spawnInterval(float elapsedTime) const {
        return std::max(0.3f, 2.0f - 0.01f * elapsedTime);
    }

    EnemyType pickType(float elapsedTime) {
        // Grunts only at first; Runners after ~1 min; Brutes after ~3 min.
        std::uniform_real_distribution<float> roll(0.f, 1.f);
        float r = roll(rng);

        if (elapsedTime > 180.f && r < 0.15f) return EnemyType::Brute;
        if (elapsedTime > 60.f && r < 0.4f) return EnemyType::Runner;
        return EnemyType::Grunt;
    }

    sf::Vector2f pickEdgePosition() {
        std::uniform_int_distribution<int> sideDist(0, 3);
        std::uniform_real_distribution<float> xDist(0.f, static_cast<float>(winSize.x));
        std::uniform_real_distribution<float> yDist(0.f, static_cast<float>(winSize.y));

        switch (sideDist(rng)) {
            case 0: return {xDist(rng), -30.f};                          // top
            case 1: return {xDist(rng), winSize.y + 30.f};                // bottom
            case 2: return {-30.f, yDist(rng)};                          // left
            default: return {winSize.x + 30.f, yDist(rng)};               // right
        }
    }

    void spawnOne(float elapsedTime, const Player& player) {
        Enemy* enemy = pool.spawn();
        if (!enemy) return; // pool exhausted, skip this spawn silently

        EnemyType type = pickType(elapsedTime);
        sf::Vector2f pos = pickEdgePosition();
        enemy->reset(type, pos, &player);
    }

    EntityPool<Enemy>& pool;
    sf::Vector2u winSize;
    std::mt19937 rng;
    float spawnTimer = 0.f;
};
