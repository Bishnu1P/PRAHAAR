#pragma once
#include <algorithm>
#include "Entity.hpp"

// Simple fading particle for death effects (no sprite/texture needed —
// just a shrinking, fading circle flying outward). Pooled like
// everything else, since a busy fight can spawn dozens per second.
class Particle : public Entity {
public:
    void reset(sf::Vector2f spawnPos, sf::Vector2f vel, sf::Color color, float lifetime) {
        setPosition(spawnPos);
        velocity = vel;
        baseColor = color;
        maxLifetime = lifetime;
        age = 0.f;
        active = true;

        shape.setRadius(7.f);
        shape.setOrigin(7.f, 7.f);
        shape.setFillColor(color);
        shape.setPosition(spawnPos);
    }

    void update(float dt) override {
        age += dt;
        position += velocity * dt;
        shape.setPosition(position);

        float ratio = std::max(0.f, 1.f - age / maxLifetime);
        sf::Color c = baseColor;
        c.a = static_cast<sf::Uint8>(255.f * ratio);
        shape.setFillColor(c);

        if (age >= maxLifetime) {
            active = false;
        }
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(shape);
    }

private:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    sf::Color baseColor;
    float age = 0.f;
    float maxLifetime = 0.4f;
};
