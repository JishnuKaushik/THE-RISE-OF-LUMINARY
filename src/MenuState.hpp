#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>
#include "Button.hpp"

class Game;

class MenuState {
private:
    Game* game;
    
    sf::Texture backgroundTexture;
    std::optional<sf::Sprite> backgroundSprite;
   
    // UI Elements
    sf::Text* title;
    std::vector<Button*> menuButtons;
    
    int selectedIndex;
    sf::Color selectedColor;
    sf::Color normalColor;
    
    // Menu options
    std::vector<std::string> options;
    
    void updateTextColors();
    void updateBoxColors();
    void cleanup();
    
public:
    MenuState(Game* gameInstance);
    ~MenuState();
    
    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    void onEnter();
    void onExit();
};
