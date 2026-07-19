#pragma once
#include <SFML/Graphics.hpp>

// Base class for anything that exists in the game world:
// player, enemies, projectiles, pickups, etc.
// Everything shares position, velocity, and an active flag for pooling.
class Entity {
public:
    virtual ~Entity() = default;

    // Called once per fixed timestep (e.g. 60 times/sec).
    // dt is the fixed timestep duration in seconds (constant every call).
    virtual void update(float dt) = 0;

    // Called once per rendered frame.
    virtual void draw(sf::RenderWindow& window) = 0;

    bool isActive() const { return active; }
    void setActive(bool value) { active = value; }

    sf::Vector2f getPosition() const { return position; }
    void setPosition(sf::Vector2f pos) { position = pos; }

protected:
    sf::Vector2f position{0.f, 0.f};
    sf::Vector2f velocity{0.f, 0.f};
    bool active = false; // inactive entities are "in the pool", skipped by update/draw
};
