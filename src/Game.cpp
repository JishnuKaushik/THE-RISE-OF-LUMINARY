#include "Game.hpp"
#include "MenuState.hpp"
#include "RegistrationState.hpp"
#include "TrainingState.hpp"
#include "ProfileSelectionState.hpp"
#include "CharacterSelectionState.hpp"
#include "SplashState.hpp"
#include "TrainingHubState.hpp"
#include "LessonViewerState.hpp"
#include "TrainingStoryState.hpp"
#include "StatisticsState.hpp"
#include "SettingsState.hpp"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

Game::Game() : isRunning(true), currentGameState(GameState::SPLASH), hasLoadedProfile(false), selectedCharacterId(1),
               fadeAlpha(0.f), fadingOut(false), fadingIn(false), fadeTimer(0.f) {
    initWindow();
    initFont();
    initStates();
}

Game::~Game() = default;

void Game::initWindow() {
    window.create(sf::VideoMode({1280, 720}), "The Rising of the Luminary");
    window.setFramerateLimit(60);
    fadeOverlay.setSize(sf::Vector2f(1280.f, 720.f));
    fadeOverlay.setPosition(sf::Vector2f(0.f, 0.f));
    fadeOverlay.setFillColor(sf::Color(0, 0, 0, 0));
    std::cout << "Window created: 1280x720" << std::endl;
}

void Game::startFade(std::function<void()> onComplete) {
    pendingStateSwitch = onComplete;
    fadingOut  = true;
    fadingIn   = false;
    fadeTimer  = 0.f;
    fadeAlpha  = 0.f;
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
    trainingStoryState = std::make_unique<TrainingStoryState>(this);
    statisticsState = std::make_unique<StatisticsState>(this);
    settingsState   = std::make_unique<SettingsState>(this);

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
    else if (currentGameState == GameState::STORY_MODE)         trainingStoryState->onExit();
    else if (currentGameState == GameState::STATISTICS)         statisticsState->onExit();
    else if (currentGameState == GameState::SETTINGS)           settingsState->onExit();
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

static std::string getTodayString() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_ptr = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm_ptr, "%Y-%m-%d");
    return oss.str();
}

void Game::switchToTraining() {
    if      (currentGameState == GameState::REGISTRATION)       registrationState->onExit();
    else if (currentGameState == GameState::PROFILE_SELECTION)  profileSelectionState->onExit();
    else if (currentGameState == GameState::CHARACTER_SELECTION) characterSelectionState->onExit();
    else if (currentGameState == GameState::TRAINING_HUB)       trainingHubState->onExit();
    else if (currentGameState == GameState::LESSON_VIEWER)      lessonViewerState->onExit();
    achievements.setUsername(playerData.username);
    // Daily reset check
    std::string today = getTodayString();
    if (playerData.lastPlayedDate != today) {
        playerData.currentStreak = (playerData.dailyQuestStreakDone) ? playerData.currentStreak + 1 : 0;
        playerData.dailyQuestionsAnswered = 0;
        playerData.dailyChaptersCompleted = 0;
        playerData.dailyQuestStreakDone   = false;
        playerData.lastPlayedDate = today;
    }
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

void Game::switchToStoryMode() {
    if (currentGameState == GameState::MENU) menuState->onExit();
    achievements.setUsername(playerData.username);
    // Daily reset check
    std::string today = getTodayString();
    if (playerData.lastPlayedDate != today) {
        playerData.currentStreak = (playerData.dailyQuestStreakDone) ? playerData.currentStreak + 1 : 0;
        playerData.dailyQuestionsAnswered = 0;
        playerData.dailyChaptersCompleted = 0;
        playerData.dailyQuestStreakDone   = false;
        playerData.lastPlayedDate = today;
    }
    currentGameState = GameState::STORY_MODE;
    trainingStoryState->onEnter();
}

void Game::switchToStatistics() {
    if (currentGameState == GameState::MENU) menuState->onExit();
    currentGameState = GameState::STATISTICS;
    statisticsState->onEnter();
}

void Game::switchToSettings() {
    if (currentGameState == GameState::MENU) menuState->onExit();
    currentGameState = GameState::SETTINGS;
    settingsState->onEnter();
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
                    currentGameState == GameState::LESSON_VIEWER ||
                    currentGameState == GameState::STORY_MODE    ||
                    currentGameState == GameState::STATISTICS     ||
                    currentGameState == GameState::SETTINGS;
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
            case GameState::STORY_MODE:
                trainingStoryState->handleInput(*event);
                break;
            case GameState::STATISTICS:
                statisticsState->handleInput(*event);
                break;
            case GameState::SETTINGS:
                settingsState->handleInput(*event);
                break;
            default:
                break;
        }
    }
}

void Game::update(float deltaTime) {
    // Screen fade update
    if (fadingOut) {
        fadeTimer += deltaTime;
        fadeAlpha = std::min(255.f, (fadeTimer / FADE_DURATION) * 255.f);
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(fadeAlpha)));
        if (fadeTimer >= FADE_DURATION) {
            if (pendingStateSwitch) { pendingStateSwitch(); pendingStateSwitch = nullptr; }
            fadingOut = false;
            fadingIn  = true;
            fadeTimer = 0.f;
        }
    } else if (fadingIn) {
        fadeTimer += deltaTime;
        fadeAlpha = std::max(0.f, 255.f - (fadeTimer / FADE_DURATION) * 255.f);
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(fadeAlpha)));
        if (fadeTimer >= FADE_DURATION) { fadingIn = false; fadeAlpha = 0.f; }
    }

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
        case GameState::STORY_MODE:
            trainingStoryState->update(deltaTime);
            break;
        case GameState::STATISTICS:
            statisticsState->update(deltaTime);
            break;
        case GameState::SETTINGS:
            settingsState->update(deltaTime);
            break;
        default:
            break;
    }
}

void Game::render() {
    // Splash manages its own clear/display cycle
    if (currentGameState == GameState::SPLASH) {
        splashState->render(window);
        return;
    }

    // All other states: clear → render state → fade overlay → display
    sf::Color bg = sf::Color(20, 15, 40);
    switch (currentGameState) {
        case GameState::TRAINING_HUB:   bg = sf::Color(14, 10, 35);  break;
        case GameState::LESSON_VIEWER:  bg = sf::Color(8,  6,  20);  break;
        case GameState::STORY_MODE:
        case GameState::STATISTICS:
        case GameState::SETTINGS:       bg = sf::Color(8,  6,  24);  break;
        default: break;
    }
    window.clear(bg);

    switch (currentGameState) {
        case GameState::MENU:               menuState->render(window);               break;
        case GameState::REGISTRATION:       registrationState->render(window);       break;
        case GameState::TRAINING:           trainingState->render(window);           break;
        case GameState::PROFILE_SELECTION:  profileSelectionState->render(window);   break;
        case GameState::CHARACTER_SELECTION:characterSelectionState->render(window); break;
        case GameState::TRAINING_HUB:       trainingHubState->render(window);        break;
        case GameState::LESSON_VIEWER:      lessonViewerState->render(window);       break;
        case GameState::STORY_MODE:         trainingStoryState->render(window);      break;
        case GameState::STATISTICS:         statisticsState->render(window);         break;
        case GameState::SETTINGS:           settingsState->render(window);           break;
        default: break;
    }

    // Fade overlay drawn on top of everything
    if (fadingOut || fadingIn)
        window.draw(fadeOverlay);

    window.display();
}
