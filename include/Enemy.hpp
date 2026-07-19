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
    // This replaces a constructor, since the object itself is reused.
    void reset(EnemyType newType, sf::Vector2f spawnPos, const Player* targetPlayer) {
        type = newType;
        target = targetPlayer;
        active = true;

        // Stats per type, matching docs/design-doc.md
        switch (type) {
            case EnemyType::Grunt:
                health = 20.f;
                speed = 80.f;
                touchDamage = 5.f;
                shape.setRadius(14.f);
                shape.setFillColor(sf::Color(200, 60, 60)); // red
                break;
            case EnemyType::Runner:
                health = 10.f;
                speed = 150.f;
                touchDamage = 3.f;
                shape.setRadius(10.f);
                shape.setFillColor(sf::Color(230, 200, 60)); // yellow
                break;
            case EnemyType::Brute:
                health = 60.f;
                speed = 50.f;
                touchDamage = 15.f;
                shape.setRadius(22.f);
                shape.setFillColor(sf::Color(120, 20, 20)); // dark red
                break;
        }

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
