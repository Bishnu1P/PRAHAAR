#pragma once
#include <cmath>
#include <algorithm>
#include "Entity.hpp"
#include "Player.hpp"

enum class BossState { Chasing, Windup, Attacking, Cooldown };

// A single boss (not pooled — only one exists at a time). Fights the
// player one-on-one after enough normal enemies have been cleared.
// Uses a simple state machine so its attack is telegraphed (brightens
// before lunging) rather than just being a bigger version of a Grunt.
class Boss : public Entity {
public:
    Boss() {
        shape.setRadius(40.f);
        shape.setOrigin(40.f, 40.f);
    }

    // stage increases each time a boss is defeated, making the next
    // one tougher (per Game's difficultyStage counter).
    void reset(sf::Vector2f spawnPos, const Player* targetPlayer, int stage) {
        target = targetPlayer;
        active = true;
        state = BossState::Chasing;
        stateTimer = 0.f;

        float scale = 1.f + 0.3f * (stage - 1);
        maxHealth = 300.f * scale;
        health = maxHealth;
        speed = 60.f;
        touchDamage = 20.f * (1.f + 0.2f * (stage - 1));

        baseColor = sf::Color(150, 20, 150);
        shape.setFillColor(baseColor);
        setPosition(spawnPos);
        shape.setPosition(spawnPos);
    }

    void update(float dt) override {
        if (!target) return;
        stateTimer += dt;

        sf::Vector2f toPlayer = target->getPosition() - position;
        float distance = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
        sf::Vector2f dir = (distance > 0.001f) ? toPlayer / distance : sf::Vector2f(0.f, 0.f);

        switch (state) {
            case BossState::Chasing:
                position += dir * speed * dt;
                if (distance <= attackRange) {
                    state = BossState::Windup;
                    stateTimer = 0.f;
                    shape.setFillColor(sf::Color(230, 100, 230)); // brighten: telegraph incoming attack
                }
                break;

            case BossState::Windup:
                // Deliberately stands still here — this pause is the
                // player's cue to dodge before the lunge comes.
                if (stateTimer >= windupDuration) {
                    state = BossState::Attacking;
                    stateTimer = 0.f;
                    lungeDir = dir;
                }
                break;

            case BossState::Attacking:
                position += lungeDir * lungeSpeed * dt;
                if (stateTimer >= attackDuration) {
                    state = BossState::Cooldown;
                    stateTimer = 0.f;
                    shape.setFillColor(baseColor);
                }
                break;

            case BossState::Cooldown:
                // Brief recovery window where the boss is vulnerable
                // and not moving much — a fair opening to land hits.
                if (stateTimer >= cooldownDuration) {
                    state = BossState::Chasing;
                    stateTimer = 0.f;
                }
                break;
        }

        shape.setPosition(position);
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(shape);
    }

    void takeDamage(float amount) {
        health = std::max(0.f, health - amount);
        if (health <= 0.f) active = false;
    }

    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getRadius() const { return shape.getRadius(); }
    float getTouchDamage() const { return touchDamage; }

private:
    sf::CircleShape shape;
    sf::Color baseColor;
    BossState state = BossState::Chasing;
    float stateTimer = 0.f;

    sf::Vector2f lungeDir;
    float speed = 0.f;
    float touchDamage = 0.f;
    float health = 0.f;
    float maxHealth = 0.f;

    const float attackRange = 150.f;
    const float windupDuration = 0.6f;
    const float attackDuration = 0.3f;
    const float cooldownDuration = 1.0f;
    const float lungeSpeed = 400.f;

    const Player* target = nullptr;
};
