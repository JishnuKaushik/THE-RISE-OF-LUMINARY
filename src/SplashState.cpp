#include "SplashState.hpp"
#include "Game.hpp"
#include <iostream>
#include<cmath>

SplashState::SplashState(Game* gameInstance) : game(gameInstance), alpha(0), timer(0), fading(true), showText(false) {
    // Load splash image
    if (!splashTexture.loadFromFile("assets/images/splash.png")) {
        std::cerr << "Failed to load splash image! Using fallback." << std::endl;
    } else {
        splashSprite.emplace(splashTexture);
        
        // Scale to fit window
        sf::Vector2u texSize = splashTexture.getSize();
        float scaleX = 1280.0f / texSize.x;
        float scaleY = 720.0f / texSize.y;
        splashSprite->setScale(sf::Vector2f(scaleX, scaleY));
    }
    
    // Load background music
    if (!backgroundMusic.openFromFile("assets/sounds/themesong.mpeg")) {
        std::cout << "No menu music found, continuing without audio" << std::endl;
    }
    
    // Press any key text
    sf::Font& font = game->getMainFont();
    pressKeyText.emplace(font);

    pressKeyText->setFont(font);
    pressKeyText->setString("PRESS ANY KEY TO BEGIN");
    pressKeyText->setCharacterSize(24);
    pressKeyText->setFont(font);
    pressKeyText->setString("PRESS ANY KEY TO BEGIN");
    pressKeyText->setCharacterSize(24);
    pressKeyText->setPosition(sf::Vector2f(500, 550));
    pressKeyText->setFillColor(sf::Color(255, 255, 255, 0));
}

SplashState::~SplashState() {
    //backgroundMusic.stop();
}

void SplashState::handleInput(const sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>()) {
        fading = false;
    }
}

void SplashState::update(float deltaTime) {
    timer += deltaTime;
    
    if (fading) {
        // Fade in
        if (alpha < 1.0f) {
            alpha += deltaTime;
            if (alpha >= 1.0f) alpha = 1.0f;
        }
        
        // Start music when fade in completes
        if (alpha >= 0.5f && backgroundMusic.getStatus() != sf::SoundSource::Status::Playing) {
            backgroundMusic.setLooping(true);
            backgroundMusic.play();
        }
        
        // Show "Press any key" after 1.5 seconds
        if (timer > 1.5f && !showText) {
            showText = true;
        }
        
        // Blink effect for text
        if (showText) {
            float blink = (sin(timer * 3) + 1) / 2;
            int blinkAlpha = static_cast<int>(blink * 200 + 55);
            pressKeyText->setFillColor(sf::Color(255, 255, 255, blinkAlpha));
        }
    } else {
        // Fade out and switch to menu
        alpha -= deltaTime * 2;
        if (alpha <= 0) {
            //backgroundMusic.stop();
            game->switchToMenu();
        }
    }
}

void SplashState::render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    if (splashSprite.has_value()) {
    window.draw(*splashSprite);
}
    
    // Draw splash image with fade
    if (splashTexture.getSize().x > 0) {
        splashSprite->setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 255)));
        window.draw(*splashSprite);
    } else {
        // Fallback text if image not found
        sf::Font& font = game->getMainFont();
        sf::Text text(font, "THE RISING OF THE LUMINARY", 40);
        text.setPosition(sf::Vector2f(400, 300));
        text.setFillColor(sf::Color(255, 215, 0, static_cast<std::uint8_t>(alpha * 255)));
        window.draw(text);
    }
    
    // Draw press key text
    if (showText && fading) {
        window.draw(*pressKeyText);
    }
    
    window.display();
}

void SplashState::onEnter() {
    std::cout << "Entered Splash State" << std::endl;
    alpha = 0;
    timer = 0;
    fading = true;
    showText = false;
}

void SplashState::onExit() {
    std::cout << "Exited Splash State" << std::endl;
}
