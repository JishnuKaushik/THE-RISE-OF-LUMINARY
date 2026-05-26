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

    backgroundTexture.emplace();
    if (!backgroundTexture->loadFromFile("assets/images/characterselect.png")) {
        std::cout << "Failed to load character selection background!" << std::endl;
    }
    backgroundSprite.emplace(*backgroundTexture);

    sf::Vector2u texSize = backgroundTexture->getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        backgroundSprite->setScale(sf::Vector2f(1280.f / texSize.x, 720.f / texSize.y));
    }

    // Invisible title placeholder — keeps cleanup() safe with a non-null pointer
    title = new sf::Text(font, "", 1);

    luminescenceText = new sf::Text(font, "LUMINESCENCE: 0", 24);
    luminescenceText->setPosition(sf::Vector2f(40, 682));
    luminescenceText->setFillColor(sf::Color(255, 215, 0));

    instructionText = new sf::Text(font, "<- ->  Navigate  |  ENTER Select  |  U  Unlock  |  ESC  Back", 20);
    instructionText->setPosition(sf::Vector2f(320, 685));
    instructionText->setFillColor(sf::Color(150, 150, 150));

    backText = new sf::Text(font, "BACK (ESC)", 20);
    backText->setPosition(sf::Vector2f(50, 50));
    backText->setFillColor(sf::Color(150, 150, 150));

    unlockText = new sf::Text(font, "", 20);
    unlockText->setPosition(sf::Vector2f(400, 650));
    unlockText->setFillColor(sf::Color::Yellow);

    std::cout << "CharacterSelectionState created" << std::endl;
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
    for (auto* box : characterBoxes) delete box;
    characterBoxes.clear();
    for (auto* sprite : cardSprites) delete sprite;
    for (auto* texture : cardTextures) delete texture;
    cardSprites.clear();
    cardTextures.clear();

    sf::Font& font = game->getMainFont();

    const float startX = 177.f;
    const float startY = 85.f;
    const float boxWidth = 207.f;
    const float boxHeight = 270.f;
    const float hSpacing = 153.f;
    const float vSpacing = 22.f;

    for (size_t i = 0; i < displayCharacters.size(); i++) {
        int row = static_cast<int>(i) / 3;
        int col = static_cast<int>(i) % 3;
        float x = startX + col * (boxWidth + hSpacing);
        float y = startY + row * (boxHeight + vSpacing);

        Character& c = displayCharacters[i];

        std::string imagePath = "assets/images/cards/" + c.spriteName;
        sf::Texture* cardTexture = new sf::Texture();

        if (!cardTexture->loadFromFile(imagePath)) {
            std::cout << "Failed to load card: " << imagePath << std::endl;
            delete cardTexture; // prevent leak — texture not pushed to owner vector
        } else {
            sf::Sprite* sprite = new sf::Sprite(*cardTexture);
            sprite->setPosition(sf::Vector2f(x, y));
            sf::Vector2u texSize = cardTexture->getSize();
            sprite->setScale(sf::Vector2f(
                boxWidth  / static_cast<float>(texSize.x),
                boxHeight / static_cast<float>(texSize.y)
            ));
            cardTextures.push_back(cardTexture);
            cardSprites.push_back(sprite);
        }

        sf::RectangleShape* box = new sf::RectangleShape(sf::Vector2f(boxWidth, boxHeight));
        box->setPosition(sf::Vector2f(x, y));
        box->setFillColor(sf::Color(0, 0, 0, 0));

        if (c.isUnlocked) {
            box->setOutlineColor(i == selectedIndex
                ? sf::Color(255, 215, 0)
                : sf::Color(150, 100, 50));
        } else {
            box->setOutlineColor(sf::Color(80, 65, 55));
        }
        box->setOutlineThickness(i == selectedIndex ? 4.f : 2.f);
        characterBoxes.push_back(box);
    }

    // Update luminescence once after all cards are processed
    int lum = characterManager->getTotalLuminescence();
    luminescenceText->setString("LUMINESCENCE: " + std::to_string(lum));
}

void CharacterSelectionState::updateColors() {
    for (size_t i = 0; i < characterBoxes.size(); i++) {
        bool selected = (i == static_cast<size_t>(selectedIndex));
        characterBoxes[i]->setOutlineThickness(selected ? 4.f : 2.f);
        characterBoxes[i]->setOutlineColor(selected
            ? sf::Color(255, 215, 0)
            : sf::Color(150, 100, 50));
    }
}

void CharacterSelectionState::selectCharacter() {
    if (selectedIndex >= static_cast<int>(displayCharacters.size())) return;

    Character& c = displayCharacters[selectedIndex];
    if (c.isUnlocked) {
        characterManager->selectCharacter(c.id);
        game->playerData.selectedCharacter = c.id;
        std::cout << "Selected character: " << c.name << std::endl;
        game->switchToTraining();
    } else {
        showMessage("Character locked! Press U to unlock (Cost: "
            + std::to_string(c.unlockCost) + " LUM)");
    }
}

void CharacterSelectionState::unlockCurrentCharacter() {
    if (selectedIndex >= static_cast<int>(displayCharacters.size())) return;

    Character& c = displayCharacters[selectedIndex];
    if (!c.isUnlocked) {
        if (characterManager->unlockCharacter(c.id)) {
            showMessage("UNLOCKED! " + c.name + " is now available!");
            loadCharacters();
            updateDisplay();
        } else {
            showMessage("Not enough Luminescence! Need " + std::to_string(c.unlockCost));
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
        switch (keyPressed->code) {
            case sf::Keyboard::Key::Left:
                if (selectedIndex > 0) {
                    selectedIndex--;
                    updateColors();
                }
                break;
            case sf::Keyboard::Key::Right:
                if (selectedIndex < static_cast<int>(displayCharacters.size()) - 1) {
                    selectedIndex++;
                    updateColors();
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
        if (messageTimer <= 0.f) {
            showUnlockMessage = false;
            unlockText->setString("");
        }
    }
}

void CharacterSelectionState::render(sf::RenderWindow& window) {
    if (backgroundSprite) window.draw(*backgroundSprite);

    if (title) window.draw(*title);

    for (auto* sprite : cardSprites) window.draw(*sprite);
    for (auto* box   : characterBoxes) window.draw(*box);

    if (luminescenceText) window.draw(*luminescenceText);
    if (instructionText)  window.draw(*instructionText);
    if (backText)         window.draw(*backText);
    if (unlockText && showUnlockMessage) window.draw(*unlockText);
}

void CharacterSelectionState::cleanup() {
    delete title;
    delete luminescenceText;
    delete instructionText;
    delete backText;
    delete unlockText;

    for (auto* box     : characterBoxes) delete box;
    for (auto* sprite  : cardSprites)    delete sprite;
    for (auto* texture : cardTextures)   delete texture;

    characterBoxes.clear();
    cardSprites.clear();
    cardTextures.clear();
    characterNames.clear();
    characterStats.clear();
}

void CharacterSelectionState::onEnter() {
    std::cout << "Entered Character Selection State" << std::endl;
    loadCharacters();
    selectedIndex = 0;
    updateDisplay();
    updateColors();
}

void CharacterSelectionState::onExit() {
    std::cout << "Exited Character Selection State" << std::endl;
    characterManager->saveProgress();
}
