#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>

class Game;

class ProfileSelectionState {
private:
    std::optional<sf::Texture> backgroundTexture;
    std::optional<sf::Sprite> backgroundSprite;

    Game* game;
    
    // UI Elements
    sf::Text* title;
    std::vector<sf::Text*> profileTexts;
    std::vector<sf::RectangleShape*> profileBoxes;
    sf::Text* instructionText;
    sf::Text* backText;
    sf::Text* deleteText;
    
    std::vector<std::string> profiles;
    int selectedIndex;
    bool deleteMode;
    
    sf::Color selectedColor;
    sf::Color normalColor;
    sf::Color deleteColor;
    
    void loadProfiles();
    void updateColors();
    void selectProfile();
    void deleteProfile();
    void cleanup();
    
public:
    ProfileSelectionState(Game* gameInstance);
    ~ProfileSelectionState();
    
    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    void onEnter();
    void onExit();
};
