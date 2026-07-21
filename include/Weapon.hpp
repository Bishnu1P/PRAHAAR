#pragma once
#include <cmath>
#include <limits>
#include "Player.hpp"
#include "Enemy.hpp"
#include "Boss.hpp"
#include "Projectile.hpp"
#include "EntityPool.hpp"
#include "PowerupSystem.hpp"

// The "Dagger" starter weapon from docs/design-doc.md: fires
// automatically at the nearest target, no manual aiming needed.
// "Nearest target" includes both regular enemies AND the boss
// (when one is active) — previously only regular enemies were
// considered, which is why the weapon ignored bosses entirely.
class Weapon {
public:
    void update(float dt, const Player& player, EntityPool<Enemy>& enemies,
                EntityPool<Projectile>& projectiles, const PowerupSystem& powerups,
                const Boss* boss, bool bossActive) {
        cooldownTimer += dt;

        float effectiveFireRate = powerups.isFireRateActive() ? fireRate * 2.f : fireRate;

        if (cooldownTimer >= 1.f / effectiveFireRate) {
            sf::Vector2f targetPos;
            bool found = findNearestTarget(player, enemies, boss, bossActive, targetPos);

            if (found) {
                fireAt(player, targetPos, projectiles, powerups);
                cooldownTimer = 0.f;
            }
        }
    }

    void increaseDamage(float amount) { damage += amount; }
    void increaseFireRate(float amount) { fireRate += amount; }

private:
    // Checks both regular enemies AND the boss, returning whichever is
    // closest. Returns false if nothing is currently targetable.
    bool findNearestTarget(const Player& player, EntityPool<Enemy>& enemies,
                            const Boss* boss, bool bossActive, sf::Vector2f& outPos) {
        bool found = false;
        float nearestDistSq = std::numeric_limits<float>::max();

        for (Enemy* enemy : enemies.getActive()) {
            sf::Vector2f diff = enemy->getPosition() - player.getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;
            if (distSq < nearestDistSq) {
                nearestDistSq = distSq;
                outPos = enemy->getPosition();
                found = true;
            }
        }

        if (bossActive && boss) {
            sf::Vector2f diff = boss->getPosition() - player.getPosition();
            float distSq = diff.x * diff.x + diff.y * diff.y;
            if (distSq < nearestDistSq) {
                nearestDistSq = distSq;
                outPos = boss->getPosition();
                found = true;
            }
        }

        return found;
    }

    void fireAt(const Player& player, sf::Vector2f targetPos, EntityPool<Projectile>& projectiles, const PowerupSystem& powerups) {
        Projectile* proj = projectiles.spawn();
        if (!proj) return;

        sf::Vector2f dir = targetPos - player.getPosition();
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (length > 0.001f) dir /= length;

        bool bigBullets = powerups.isBigBulletsActive();
        float effectiveDamage = bigBullets ? damage * 2.5f : damage;
        float radiusMultiplier = bigBullets ? 2.5f : 1.f;

        proj->reset(player.getPosition(), dir, projectileSpeed, effectiveDamage, radiusMultiplier);
    }

    float fireRate = 1.5f;
    float damage = 8.f;
    float projectileSpeed = 500.f;
    float cooldownTimer = 0.f;
};
