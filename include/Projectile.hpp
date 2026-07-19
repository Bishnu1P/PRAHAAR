#pragma once
#include "Entity.hpp"

// A single pooled projectile fired by a weapon. Travels in a straight
// line and deactivates itself after a set lifetime (so stray shots
// that miss don't fly forever).
class Projectile : public Entity {
public:
    void reset(sf::Vector2f startPos, sf::Vector2f dir, float projSpeed, float dmg) {
        setPosition(startPos);
        shape.setRadius(4.f);
        shape.setOrigin(4.f, 4.f);
        shape.setFillColor(sf::Color(230, 230, 120));
        shape.setPosition(startPos);

        direction = dir;
        speed = projSpeed;
        damage = dmg;
        age = 0.f;
        active = true;
    }

    void update(float dt) override {
        position += direction * speed * dt;
        shape.setPosition(position);

        age += dt;
        if (age >= maxLifetime) {
            active = false; // expire; slot returns to the pool automatically
        }
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(shape);
    }

    float getDamage() const { return damage; }
    float getRadius() const { return shape.getRadius(); }

private:
    sf::CircleShape shape;
    sf::Vector2f direction;
    float speed = 0.f;
    float damage = 0.f;
    float age = 0.f;
    float maxLifetime = 1.5f; // seconds before a missed shot despawns
};
