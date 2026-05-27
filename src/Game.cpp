#include "Game.hpp"
#include "MenuState.hpp"
#include "RegistrationState.hpp"
#include "TrainingState.hpp"
#include "ProfileSelectionState.hpp"
#include "CharacterSelectionState.hpp"
#include "SplashState.hpp"
#include "TrainingHubState.hpp"
#include "LessonViewerState.hpp"

Game::Game() : isRunning(true), currentGameState(GameState::SPLASH), hasLoadedProfile(false), selectedCharacterId(1) {
    initWindow();
    initFont();
    initStates();
}

Game::~Game() = default;

void Game::initWindow() {
    window.create(sf::VideoMode({1280, 720}), "The Rising of the Luminary");
    window.setFramerateLimit(60);
    std::cout << "Window created: 1280x720" << std::endl;
}

void Game::initFont() {
    std::vector<std::string> fontPaths = {
        "assets/fonts/arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
    };
    
    for (const auto& path : fontPaths) {
        if (mainFont.openFromFile(path)) {
            std::cout << "Font loaded from: " << path << std::endl;
            break;
        }
    }
}

void Game::initStates() {
    splashState = std::make_unique<SplashState>(this);
    menuState = std::make_unique<MenuState>(this);
    registrationState = std::make_unique<RegistrationState>(this);
    trainingState = std::make_unique<TrainingState>(this);
    profileSelectionState = std::make_unique<ProfileSelectionState>(this);
    characterSelectionState = std::make_unique<CharacterSelectionState>(this);
    trainingHubState = std::make_unique<TrainingHubState>(this);
    lessonViewerState = std::make_unique<LessonViewerState>(this);

    splashState->onEnter();
}

void Game::switchToSplash() {
    if (currentGameState == GameState::MENU) {
        menuState->onExit();
    }
    currentGameState = GameState::SPLASH;
    splashState->onEnter();
}

void Game::switchToMenu() {
    if      (currentGameState == GameState::SPLASH)             splashState->onExit();
    else if (currentGameState == GameState::REGISTRATION)       registrationState->onExit();
    else if (currentGameState == GameState::TRAINING)           trainingState->onExit();
    else if (currentGameState == GameState::PROFILE_SELECTION)  profileSelectionState->onExit();
    else if (currentGameState == GameState::CHARACTER_SELECTION) characterSelectionState->onExit();
    else if (currentGameState == GameState::TRAINING_HUB)       trainingHubState->onExit();
    else if (currentGameState == GameState::LESSON_VIEWER)      lessonViewerState->onExit();
    currentGameState = GameState::MENU;
    menuState->onEnter();
    std::cout << "Switched to Menu" << std::endl;
}

void Game::switchToRegistration() {
    if (currentGameState == GameState::MENU) {
        menuState->onExit();
    }
    currentGameState = GameState::REGISTRATION;
    registrationState->onEnter();
}

void Game::switchToTraining() {
    if      (currentGameState == GameState::REGISTRATION)       registrationState->onExit();
    else if (currentGameState == GameState::PROFILE_SELECTION)  profileSelectionState->onExit();
    else if (currentGameState == GameState::CHARACTER_SELECTION) characterSelectionState->onExit();
    else if (currentGameState == GameState::TRAINING_HUB)       trainingHubState->onExit();
    else if (currentGameState == GameState::LESSON_VIEWER)      lessonViewerState->onExit();
    currentGameState = GameState::TRAINING;
    trainingState->onEnter();
}

void Game::switchToTrainingHub() {
    if      (currentGameState == GameState::MENU)          menuState->onExit();
    else if (currentGameState == GameState::TRAINING)      trainingState->onExit();
    else if (currentGameState == GameState::LESSON_VIEWER) lessonViewerState->onExit();
    currentGameState = GameState::TRAINING_HUB;
    trainingHubState->onEnter();
}

void Game::switchToLessonViewer() {
    if (currentGameState == GameState::TRAINING_HUB) trainingHubState->onExit();
    lessonViewerState->loadPages(trainingChapterPath, trainingChapterTitle);
    currentGameState = GameState::LESSON_VIEWER;
    lessonViewerState->onEnter();
}

void Game::switchToProfileSelection() {
    if (currentGameState == GameState::MENU) {
        menuState->onExit();
    }
    currentGameState = GameState::PROFILE_SELECTION;
    profileSelectionState->onEnter();
}

void Game::switchToCharacterSelection() {
    if (currentGameState == GameState::REGISTRATION) {
        registrationState->onExit();
    }
    currentGameState = GameState::CHARACTER_SELECTION;
    characterSelectionState->onEnter();
}

void Game::switchToGameplay() {
    currentGameState = GameState::GAMEPLAY;
}

void Game::run() {
    std::cout << "Game is running!" << std::endl;
    sf::Clock clock;
    
    while (window.isOpen() && isRunning) {
        float deltaTime = clock.restart().asSeconds();
        
        handleEvents();
        update(deltaTime);
        render();
    }
    
    std::cout << "Game closed" << std::endl;
}

void Game::handleEvents() {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            isRunning = false;
        }
        
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                // TRAINING_HUB and LESSON_VIEWER handle ESC internally
                bool stateHandlesOwnEsc =
                    currentGameState == GameState::TRAINING_HUB ||
                    currentGameState == GameState::LESSON_VIEWER;
                if (!stateHandlesOwnEsc) {
                    if (currentGameState != GameState::MENU && currentGameState != GameState::SPLASH) {
                        switchToMenu();
                    } else if (currentGameState == GameState::MENU) {
                        window.close();
                        isRunning = false;
                    }
                }
            }
        }
        
        switch (currentGameState) {
            case GameState::SPLASH:
                splashState->handleInput(*event);
                break;
            case GameState::MENU:
                menuState->handleInput(*event);
                break;
            case GameState::REGISTRATION:
                registrationState->handleInput(*event);
                break;
            case GameState::TRAINING:
                trainingState->handleInput(*event);
                break;
            case GameState::PROFILE_SELECTION:
                profileSelectionState->handleInput(*event);
                break;
            case GameState::CHARACTER_SELECTION:
                characterSelectionState->handleInput(*event);
                break;
            case GameState::TRAINING_HUB:
                trainingHubState->handleInput(*event);
                break;
            case GameState::LESSON_VIEWER:
                lessonViewerState->handleInput(*event);
                break;
            default:
                break;
        }
    }
}

void Game::update(float deltaTime) {
    switch (currentGameState) {
        case GameState::SPLASH:
            splashState->update(deltaTime);
            break;
        case GameState::MENU:
            menuState->update(deltaTime);
            break;
        case GameState::REGISTRATION:
            registrationState->update(deltaTime);
            break;
        case GameState::TRAINING:
            trainingState->update(deltaTime);
            break;
        case GameState::PROFILE_SELECTION:
            profileSelectionState->update(deltaTime);
            break;
        case GameState::CHARACTER_SELECTION:
            characterSelectionState->update(deltaTime);
            break;
        case GameState::TRAINING_HUB:
            trainingHubState->update(deltaTime);
            break;
        case GameState::LESSON_VIEWER:
            lessonViewerState->update(deltaTime);
            break;
        default:
            break;
    }
}

void Game::render() {
    switch (currentGameState) {
        case GameState::SPLASH:
            splashState->render(window);
            break;
        case GameState::MENU:
            window.clear(sf::Color(20, 15, 40));
            menuState->render(window);
            window.display();
            break;
        case GameState::REGISTRATION:
            window.clear(sf::Color(20, 15, 40));
            registrationState->render(window);
            window.display();
            break;
        case GameState::TRAINING:
            window.clear(sf::Color(20, 15, 40));
            trainingState->render(window);
            window.display();
            break;
        case GameState::PROFILE_SELECTION:
            window.clear(sf::Color(20, 15, 40));
            profileSelectionState->render(window);
            window.display();
            break;
        case GameState::CHARACTER_SELECTION:
            window.clear(sf::Color(20, 15, 40));
            characterSelectionState->render(window);
            window.display();
            break;
        case GameState::TRAINING_HUB:
            window.clear(sf::Color(14, 10, 35));
            trainingHubState->render(window);
            window.display();
            break;
        case GameState::LESSON_VIEWER:
            window.clear(sf::Color(8, 6, 20));
            lessonViewerState->render(window);
            window.display();
            break;
        default:
            window.clear(sf::Color::Black);
            window.display();
            break;
    }
}
