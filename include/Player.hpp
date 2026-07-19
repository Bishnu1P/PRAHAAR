#pragma once
#include <cmath>
#include <algorithm>
#include "Entity.hpp"

// Player character. Green circle placeholder art, WASD movement,
// and health tracking (100 HP start, per docs/design-doc.md).
class Player : public Entity {
public:
    Player() {
        shape.setRadius(20.f);
        shape.setFillColor(sf::Color::Green);
        shape.setOrigin(20.f, 20.f);
        setPosition({400.f, 300.f});
        speed = 200.f;
        active = true;

        maxHealth = 100.f;
        health = maxHealth;
    }

    void update(float dt) override {
        velocity = {0.f, 0.f};

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            velocity.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            velocity.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            velocity.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            velocity.x += 1.f;

        float length = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (length != 0.f) {
            velocity /= length;
        }

        position += velocity * speed * dt;
        shape.setPosition(position);

        if (invulnerableTimer > 0.f) {
            invulnerableTimer -= dt;
        }
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(shape);
        drawHealthBar(window);
    }

    // --- Health system ---
    // A short invulnerability window after each hit prevents an enemy
    // standing on top of the player from draining full HP in one frame.
    void takeDamage(float amount) {
        if (invulnerableTimer > 0.f) return;
        health = std::max(0.f, health - amount);
        invulnerableTimer = 0.5f; // half a second of grace after each hit
    }

    void heal(float amount) {
        health = std::min(maxHealth, health + amount);
    }

    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    bool isAlive() const { return health > 0.f; }
    float getRadius() const { return shape.getRadius(); }
    float getPickupRadius() const { return pickupRadius; }

    // --- Upgrades (applied on level-up, see Progression/Game) ---
    void increaseMaxHealth(float amount) {
        maxHealth += amount;
        health += amount; // levelling up also heals by the bonus amount
    }

    void increaseSpeed(float amount) {
        speed += amount;
    }

private:
    void drawHealthBar(sf::RenderWindow& window) {
        float barWidth = 40.f;
        float barHeight = 5.f;
        float ratio = (maxHealth > 0.f) ? (health / maxHealth) : 0.f;

        sf::Vector2f barPos = position + sf::Vector2f(-barWidth / 2.f, -shape.getRadius() - 12.f);

        sf::RectangleShape background(sf::Vector2f(barWidth, barHeight));
        background.setPosition(barPos);
        background.setFillColor(sf::Color(60, 60, 60));

        sf::RectangleShape fill(sf::Vector2f(barWidth * ratio, barHeight));
        fill.setPosition(barPos);
        fill.setFillColor(sf::Color(220, 50, 50));

        window.draw(background);
        window.draw(fill);
    }

    sf::CircleShape shape;
    float speed;

    float health;
    float maxHealth;
    float invulnerableTimer = 0.f;
    float pickupRadius = 40.f; // matches docs/design-doc.md
};
