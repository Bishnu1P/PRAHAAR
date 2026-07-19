#pragma once
#include <cmath>
#include "Entity.hpp"
#include "Player.hpp"

// XP orb dropped by dead enemies. Sits still until the player gets
// within pickup range, then flies toward them (the "magnet" effect
// common in this genre). Actual collection/XP-award happens in
// Game::handleCollisions, not here — this class only handles movement.
class XPOrb : public Entity {
public:
    void reset(sf::Vector2f spawnPos, float xpValue, const Player* targetPlayer) {
        setPosition(spawnPos);
        value = xpValue;
        target = targetPlayer;
        active = true;

        shape.setRadius(5.f);
        shape.setOrigin(5.f, 5.f);
        shape.setFillColor(sf::Color(80, 220, 230)); // cyan
        shape.setPosition(spawnPos);
    }

    void update(float dt) override {
        if (!target) return;

        sf::Vector2f toPlayer = target->getPosition() - position;
        float distance = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

        if (distance < target->getPickupRadius() && distance > 0.001f) {
            toPlayer /= distance;
            position += toPlayer * magnetSpeed * dt;
            shape.setPosition(position);
        }
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(shape);
    }

    float getValue() const { return value; }
    float getRadius() const { return shape.getRadius(); }

private:
    sf::CircleShape shape;
    float value = 0.f;
    float magnetSpeed = 300.f;
    const Player* target = nullptr;
};
