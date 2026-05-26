#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>

class Game;

class RegistrationState {
private:
    std::optional<sf::Texture> backgroundTexture;
    std::optional<sf::Sprite> backgroundSprite;
    Game* game;
    
    // UI Elements
    sf::Text* title;
    sf::Text* namePrompt;
    sf::Text* nameInput;
    sf::Text* agePrompt;
    sf::Text* ageInput;
    sf::Text* gradePrompt;
    sf::Text* gradeDisplay;
    sf::Text* subjectPrompt;
    std::vector<sf::Text*> subjectTexts;
    sf::Text* instructionText;
    
    std::vector<std::string> subjects;
    std::vector<std::string> gradeLevels;
    
    int selectedIndex;
    sf::Color selectedColor;
    sf::Color normalColor;
    
    // Input handling
    std::string usernameInputText;
    std::string ageInputText;
    bool isTypingName;
    bool isTypingAge;
    
    void updateTextColors();
    void updateGradeLevel();
    void saveProfile();
    void cleanup();
    
public:
    RegistrationState(Game* gameInstance);
    ~RegistrationState();
    
    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    void onEnter();
    void onExit();
};
