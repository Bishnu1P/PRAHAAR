#pragma once
#include "Entity.hpp"
#include "Player.hpp"
#include <cmath>
#include <algorithm>

enum class EnemyType { Grunt, Runner, Brute };

// A single pooled enemy. Objects of this class get reused (reset())
// rather than destroyed/recreated, per the EntityPool design from Step 2.
class Enemy : public Entity {
public:
    Enemy() {
        shape.setOrigin(0.f, 0.f); // set properly in reset() once radius is known
    }

    // Called every time this pooled slot is (re)spawned as a new enemy.
    // difficultyMultiplier scales health/damage/speed up as the game
    // gets harder (combines elapsed survival time + player level —
    // see EnemySpawner for how it's computed).
    void reset(EnemyType newType, sf::Vector2f spawnPos, const Player* targetPlayer, float difficultyMultiplier = 1.f) {
        type = newType;
        target = targetPlayer;
        active = true;

        // Base stats per type, matching docs/design-doc.md
        float baseHealth, baseSpeed, baseDamage;
        switch (type) {
            case EnemyType::Grunt:
                baseHealth = 20.f; baseSpeed = 80.f; baseDamage = 5.f;
                shape.setRadius(14.f);
                shape.setFillColor(sf::Color(200, 60, 60)); // red
                break;
            case EnemyType::Runner:
                baseHealth = 10.f; baseSpeed = 150.f; baseDamage = 3.f;
                shape.setRadius(10.f);
                shape.setFillColor(sf::Color(230, 200, 60)); // yellow
                break;
            default: // Brute
                baseHealth = 60.f; baseSpeed = 50.f; baseDamage = 15.f;
                shape.setRadius(22.f);
                shape.setFillColor(sf::Color(120, 20, 20)); // dark red
                break;
        }

        health = baseHealth * difficultyMultiplier;
        touchDamage = baseDamage * difficultyMultiplier;
        // Speed scales more gently than health/damage — a swarm that's
        // both tankier, harder-hitting, AND much faster gets unfair fast.
        speed = baseSpeed * (1.f + (difficultyMultiplier - 1.f) * 0.3f);

        shape.setOrigin(shape.getRadius(), shape.getRadius());
        setPosition(spawnPos);
        shape.setPosition(spawnPos);
    }

    void update(float dt) override {
        if (!target) return;

        // Basic pursuit: move directly toward the player's current position.
        sf::Vector2f toPlayer = target->getPosition() - position;
        float distance = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

        if (distance > 0.001f) {
            toPlayer /= distance; // normalize
            position += toPlayer * speed * dt;
            shape.setPosition(position);
        }
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(shape);
    }

    // --- Combat ---
    void takeDamage(float amount) {
        health = std::max(0.f, health - amount);
        if (health <= 0.f) {
            active = false; // dies -> slot returns to the pool automatically
        }
    }

    float getHealth() const { return health; }
    float getRadius() const { return shape.getRadius(); }
    float getTouchDamage() const { return touchDamage; }

private:
    sf::CircleShape shape;
    EnemyType type = EnemyType::Grunt;
    float speed = 0.f;
    float health = 0.f;
    float touchDamage = 0.f;
    const Player* target = nullptr;
};
