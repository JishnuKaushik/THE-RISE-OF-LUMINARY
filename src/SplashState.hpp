#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
class Game;

class SplashState {
private:
    Game* game;
    sf::Texture splashTexture;
    std::optional<
sf::Sprite> splashSprite;
    std::optional<sf::Text> pressKeyText;
    sf::Music backgroundMusic;
    float alpha;
    float timer;
    bool fading;
    bool showText;
    
public:
    SplashState(Game* gameInstance);
    ~SplashState();
    
    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    void onEnter();
    void onExit();
};
