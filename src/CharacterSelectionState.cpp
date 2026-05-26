#include "CharacterSelectionState.hpp"
#include "Game.hpp"
#include "SaveManager.hpp"
#include <iostream>
#include <sstream>

CharacterSelectionState::CharacterSelectionState(Game* gameInstance) 
    : game(gameInstance), selectedIndex(0), showUnlockMessage(false), messageTimer(0) {
    
    selectedColor = sf::Color::Yellow;
    normalColor = sf::Color::White;
    lockedColor = sf::Color(150, 150, 150);
    unlockedColor = sf::Color(100, 200, 100);
    
    characterManager = new CharacterManager();
    loadCharacters();
    
    sf::Font& font = game->getMainFont();
    
    // Load character selection background
    backgroundTexture.emplace();
    if (!backgroundTexture->loadFromFile("assets/images/character_bg.png")) {
        std::cout << "Failed to load character selection background! Using fallback." << std::endl;
    } else {
        backgroundSprite.emplace(*backgroundTexture);
        sf::Vector2u texSize = backgroundTexture->getSize();
        backgroundSprite->setScale(
            sf::Vector2f(
                1280.f / texSize.x,
                720.f / texSize.y
            )
        );
    }
    
    // Title - commented out to let background show
    // title = new sf::Text(font, "SELECT YOUR CHAMPION", 48);
    // title->setFillColor(sf::Color::Yellow);
    // title->setOutlineThickness(2);
    // title->setOutlineColor(sf::Color::Black);
    // title->setPosition(sf::Vector2f(420, 50));
    title = nullptr;
    
    // Luminescence display
    luminescenceText = new sf::Text(font, "LUMINESCENCE: 1000", 24);
    luminescenceText->setPosition(sf::Vector2f(50, 680));
    luminescenceText->setFillColor(sf::Color(255, 215, 0));
    
    // Instructions for number keys
    instructionText = new sf::Text(font, "PRESS 1-6 TO SELECT CHARACTER  |  U = UNLOCK  |  ESC = BACK", 20);
    instructionText->setPosition(sf::Vector2f(380, 680));
    instructionText->setFillColor(sf::Color(150, 150, 150));
    
    backText = new sf::Text(font, "← BACK (ESC)", 20);
    backText->setPosition(sf::Vector2f(50, 50));
    backText->setFillColor(sf::Color(150, 150, 150));
    
    unlockText = new sf::Text(font, "", 20);
    unlockText->setPosition(sf::Vector2f(400, 650));
    unlockText->setFillColor(sf::Color::Yellow);
    
    // Create character display boxes (commented out - using background only)
    // updateDisplay();
    
    std::cout << "CharacterSelectionState created with " << displayCharacters.size() << " characters" << std::endl;
}

CharacterSelectionState::~CharacterSelectionState() {
    cleanup();
    delete characterManager;
}

void CharacterSelectionState::loadCharacters() {
    displayCharacters = characterManager->getUnlockedCharacters();
    auto locked = characterManager->getLockedCharacters();
    
    for (auto& c : locked) {
        displayCharacters.push_back(c);
    }
}

void CharacterSelectionState::updateDisplay() {
    // DISABLED - Using background image instead of drawing boxes
    // This function is kept empty to prevent rendering boxes
}

void CharacterSelectionState::updateColors() {
    // DISABLED - No boxes to update
}

void CharacterSelectionState::selectCharacter() {
    if (selectedIndex >= (int)displayCharacters.size()) return;
    
    Character& c = displayCharacters[selectedIndex];
    if (c.isUnlocked) {
        characterManager->selectCharacter(c.id);
        game->playerData.selectedCharacter = c.id;
        std::cout << "Selected character: " << c.name << std::endl;
        game->switchToTraining();
    } else {
        showMessage("⚠️ Character locked! Press U to unlock (Cost: " + std::to_string(c.unlockCost) + " LUM)");
    }
}

void CharacterSelectionState::unlockCurrentCharacter() {
    if (selectedIndex >= (int)displayCharacters.size()) return;
    
    Character& c = displayCharacters[selectedIndex];
    if (!c.isUnlocked) {
        if (characterManager->unlockCharacter(c.id)) {
            showMessage("✅ UNLOCKED! " + c.name + " is now available!");
            loadCharacters();
            updateDisplay();
        } else {
            showMessage("❌ Not enough Luminescence! Need " + std::to_string(c.unlockCost));
        }
    }
}

void CharacterSelectionState::showMessage(const std::string& msg) {
    showUnlockMessage = true;
    messageTimer = 2.0f;
    unlockMessage = msg;
    unlockText->setString(msg);
}

void CharacterSelectionState::handleInput(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        // Number keys 1-6 for character selection
        int keyNum = -1;
        switch (keyPressed->code) {
            case sf::Keyboard::Key::Num1: keyNum = 0; break;
            case sf::Keyboard::Key::Num2: keyNum = 1; break;
            case sf::Keyboard::Key::Num3: keyNum = 2; break;
            case sf::Keyboard::Key::Num4: keyNum = 3; break;
            case sf::Keyboard::Key::Num5: keyNum = 4; break;
            case sf::Keyboard::Key::Num6: keyNum = 5; break;
            default: break;
        }
        
        if (keyNum >= 0 && keyNum < (int)displayCharacters.size()) {
            selectedIndex = keyNum;
            selectCharacter();
            return;
        }
        
        switch (keyPressed->code) {
            case sf::Keyboard::Key::Left:
                if (selectedIndex > 0) {
                    selectedIndex--;
                }
                break;
            case sf::Keyboard::Key::Right:
                if (selectedIndex < (int)displayCharacters.size() - 1) {
                    selectedIndex++;
                }
                break;
            case sf::Keyboard::Key::Enter:
                selectCharacter();
                break;
            case sf::Keyboard::Key::U:
                unlockCurrentCharacter();
                break;
            case sf::Keyboard::Key::Escape:
                game->switchToRegistration();
                break;
            default:
                break;
        }
    }
}

void CharacterSelectionState::update(float deltaTime) {
    if (showUnlockMessage) {
        messageTimer -= deltaTime;
        if (messageTimer <= 0) {
            showUnlockMessage = false;
            unlockText->setString("");
        }
    }
}

void CharacterSelectionState::render(sf::RenderWindow& window) {
    // Draw only background and UI text
    if (backgroundSprite) {
        window.draw(*backgroundSprite);
    } else {
        window.clear(sf::Color(20, 15, 40));
    }
    
    // Draw UI text elements only (no boxes)
    if (luminescenceText) window.draw(*luminescenceText);
    if (instructionText) window.draw(*instructionText);
    if (backText) window.draw(*backText);
    if (unlockText && showUnlockMessage) window.draw(*unlockText);
}

void CharacterSelectionState::cleanup() {
    if (title) delete title;
    delete luminescenceText;
    delete instructionText;
    delete backText;
    delete unlockText;
    for (auto* text : characterNames) delete text;
    for (auto* text : characterStats) delete text;
    for (auto* box : characterBoxes) delete box;
    characterNames.clear();
    characterStats.clear();
    characterBoxes.clear();
}

void CharacterSelectionState::onEnter() {
    std::cout << "Entered Character Selection State" << std::endl;
    loadCharacters();
    selectedIndex = 0;
}

void CharacterSelectionState::onExit() {
    std::cout << "Exited Character Selection State" << std::endl;
    characterManager->saveProgress();
}
