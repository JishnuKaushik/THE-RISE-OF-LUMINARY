#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Character.hpp"
#include <optional>

class Game;

class CharacterSelectionState {
private:
std::vector<sf::Texture*> cardTextures;
std::vector<sf::Sprite*> cardSprites;
std::optional<sf::Texture> backgroundTexture;
std::optional<sf::Sprite> backgroundSprite;

    Game* game;
    CharacterManager* characterManager;
    
    // UI Elements
    sf::Text* title;
    std::vector<sf::Text*> characterNames;
    std::vector<sf::Text*> characterStats;
    std::vector<sf::RectangleShape*> characterBoxes;
    sf::Text* luminescenceText;
    sf::Text* instructionText;
    sf::Text* backText;
    sf::Text* unlockText;
    
    std::vector<Character> displayCharacters;
    int selectedIndex;
    bool showUnlockMessage;
    float messageTimer;
    std::string unlockMessage;
    
    sf::Color selectedColor;
    sf::Color normalColor;
    sf::Color lockedColor;
    sf::Color unlockedColor;
    
    void loadCharacters();
    void updateDisplay();
    void updateColors();
    void selectCharacter();
    void unlockCurrentCharacter();
    void showMessage(const std::string& msg);
    void cleanup();
    
public:
    CharacterSelectionState(Game* gameInstance);
    ~CharacterSelectionState();
    
    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    void onEnter();
    void onExit();
};
