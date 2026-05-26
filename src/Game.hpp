#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include "SaveManager.hpp"

class MenuState;
class RegistrationState;
class TrainingState;
class ProfileSelectionState;
class CharacterSelectionState;
class SplashState;

class Game {
private:
    sf::RenderWindow window;
    sf::Font mainFont;
    bool isRunning;
    
    std::unique_ptr<MenuState> menuState;
    std::unique_ptr<RegistrationState> registrationState;
    std::unique_ptr<TrainingState> trainingState;
    std::unique_ptr<ProfileSelectionState> profileSelectionState;
    std::unique_ptr<CharacterSelectionState> characterSelectionState;
    std::unique_ptr<SplashState> splashState;
    
    enum class GameState { SPLASH, MENU, REGISTRATION, TRAINING, GAMEPLAY, PROFILE_SELECTION, CHARACTER_SELECTION };
    GameState currentGameState;
    
    void initWindow();
    void initFont();
    void initStates();
    
public:
    Game();
    ~Game();
    
    void run();
    void handleEvents();
    void update(float deltaTime);
    void render();
    
    void switchToSplash();
    void switchToMenu();
    void switchToRegistration();
    void switchToTraining();
    void switchToGameplay();
    void switchToProfileSelection();
    void switchToCharacterSelection();
    
    sf::RenderWindow& getWindow() { return window; }
    sf::Font& getMainFont() { return mainFont; }
    
    std::string selectedSubject;
    PlayerData playerData;
    bool hasLoadedProfile;
    int selectedCharacterId;
};
