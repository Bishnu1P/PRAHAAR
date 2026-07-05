#include <SFML/Graphics.hpp>

int main() {
    // 1. Create the game window (800x600 pixels)
    sf::RenderWindow window(sf::VideoMode(800, 600), "Prahaar: Vampire Survivors Clone");
    window.setFramerateLimit(60); // Keeps the game running smoothly at 60 FPS

    // 2. Create the Player Character (A green circle for now)
    sf::CircleShape player(20.f); // Radius of 20 pixels
    player.setFillColor(sf::Color::Green);
    player.setOrigin(20.f, 20.f); // Set origin to center of the circle
    player.setPosition(400.f, 300.f); // Start right in the middle of the screen

    // Player speed (pixels per frame)
    float playerSpeed = 5.f;

    // 3. The Main Game Loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // 4. Handle Keyboard Input for Movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            player.move(0.f, -playerSpeed); // Move Up (Y goes down in games!)
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            player.move(0.f, playerSpeed);  // Move Down
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            player.move(-playerSpeed, 0.f); // Move Left
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            player.move(playerSpeed, 0.f);  // Move Right
        }

        // 5. Rendering (Drawing everything to the screen)
        window.clear(sf::Color(30, 30, 30)); // Clear screen with a dark grey background
        
        window.draw(player); // Draw our green player character
        
        window.display(); // Swap the background buffer to display it on screen
    }

    return 0;
}