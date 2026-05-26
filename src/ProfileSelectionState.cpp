#include "ProfileSelectionState.hpp"
#include "Game.hpp"
#include "SaveManager.hpp"
#include <iostream>

ProfileSelectionState::ProfileSelectionState(Game* gameInstance) : game(gameInstance), selectedIndex(0), deleteMode(false) {
    selectedColor = sf::Color::Yellow;
    normalColor = sf::Color::White;
    deleteColor = sf::Color::Red;
    
    sf::Font& font = game->getMainFont();
    backgroundTexture.emplace();

if (backgroundTexture->loadFromFile("assets/images/loadgame.png")) {

    backgroundSprite.emplace(*backgroundTexture);

    sf::Vector2u texSize = backgroundTexture->getSize();

    backgroundSprite->setScale(
    sf::Vector2f(
        1280.f / texSize.x,
        720.f / texSize.y
    )
  );
}
    
    // Title
title = new sf::Text(font, "SELECT            PROFILE", 25);

title->setPosition(sf::Vector2f(502, 88));

title->setFillColor(sf::Color(225, 195, 120));


// Main instruction
instructionText = new sf::Text(
    font,
    "ENTER = SELECT",
    16
);

instructionText->setPosition(sf::Vector2f(420, 688));

instructionText->setFillColor(sf::Color(165, 155, 140));


// Back text
backText = new sf::Text(font, "BACK [ESC]", 16);

backText->setPosition(sf::Vector2f(40, 688));

backText->setFillColor(sf::Color(145, 140, 130));


// Delete text
deleteText = new sf::Text(font, "DELETE PROFILE [D]", 16);

deleteText->setPosition(sf::Vector2f(980, 688));

deleteText->setFillColor(sf::Color(185, 120, 120));
    
    
    loadProfiles();
    std::cout << "ProfileSelectionState created with " << profiles.size() << " profiles" << std::endl;
}

ProfileSelectionState::~ProfileSelectionState() {
    cleanup();
}

void ProfileSelectionState::loadProfiles() {
    profiles = SaveManager::getSavedProfiles();
    updateColors();
}

void ProfileSelectionState::updateColors() {
    // Clear existing UI elements
    for (auto* text : profileTexts) delete text;
    for (auto* box : profileBoxes) delete box;
    profileTexts.clear();
    profileBoxes.clear();
    
    sf::Font& font = game->getMainFont();
    

    float startY = 170;

    for (size_t i = 0; i < profiles.size(); i++) {

        sf::RectangleShape* box =
            new sf::RectangleShape(
                sf::Vector2f(420, 56)
            );

        box->setPosition(
            sf::Vector2f(
                430,
                startY + i * 72
            )
        );

        if (i == selectedIndex) {

            box->setFillColor(
                sf::Color(35, 45, 75, 220)
            );

            box->setOutlineColor(
                sf::Color(255, 220, 100)
            );

            box->setOutlineThickness(3.f);

        } else {

            box->setFillColor(
                sf::Color(12, 18, 38, 180)
            );

            box->setOutlineColor(
                sf::Color(170, 135, 65, 170)
            );

            box->setOutlineThickness(2.f);
        }

        profileBoxes.push_back(box);

        sf::Text* profileText =
            new sf::Text(
                font,
                profiles[i],
                23
            );

        profileText->setPosition(
            sf::Vector2f(
                462,
                startY + i * 72 + 12
            )
        );

        profileText->setFillColor(
            i == selectedIndex
            ? sf::Color(255, 235, 150)
            : sf::Color(220, 220, 225)
        );

        profileTexts.push_back(profileText);
    }
}

void ProfileSelectionState::selectProfile() {
    if (selectedIndex >= 0 && selectedIndex < (int)profiles.size()) {
        PlayerData data;
        if (SaveManager::loadPlayer(profiles[selectedIndex], data)) {
            game->playerData = data;
            game->selectedSubject = data.selectedSubject;
            game->hasLoadedProfile = true;
            std::cout << "Loaded profile: " << data.username << std::endl;
            game->switchToTraining();
        }
    }
}

void ProfileSelectionState::deleteProfile() {
    if (deleteMode && selectedIndex >= 0 && selectedIndex < (int)profiles.size()) {
        SaveManager::deleteProfile(profiles[selectedIndex]);
        std::cout << "Deleted profile: " << profiles[selectedIndex] << std::endl;
        loadProfiles();
        deleteMode = false;
        
        if (profiles.empty()) {
            game->switchToMenu();
        } else if (selectedIndex >= (int)profiles.size()) {
            selectedIndex = profiles.size() - 1;
            updateColors();
        }
    }
}

void ProfileSelectionState::handleInput(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        
        if (keyPressed->code == sf::Keyboard::Key::Up) {
            if (!profiles.empty()) {
                selectedIndex = (selectedIndex - 1 + profiles.size()) % profiles.size();
                updateColors();
                std::cout << "Selected: " << profiles[selectedIndex] << std::endl;
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Down) {
            if (!profiles.empty()) {
                selectedIndex = (selectedIndex + 1) % profiles.size();
                updateColors();
                std::cout << "Selected: " << profiles[selectedIndex] << std::endl;
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Enter) {
            if (!profiles.empty()) {
                if (deleteMode) {
                    deleteProfile();
                } else {
                    selectProfile();
                }
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::D) {
            if (!profiles.empty()) {
                deleteMode = !deleteMode;
                std::cout << "Delete mode: " << (deleteMode ? "ON" : "OFF") << std::endl;
                
                // Update visual feedback
                if (deleteMode) {
                    deleteText->setFillColor(sf::Color::Red);
                    deleteText->setString("PRESS ENTER TO DELETE");
                } else {
                    deleteText->setFillColor(sf::Color(200, 100, 100));
                    deleteText->setString("PRESS D TO DELETE SELECTED PROFILE");
                }
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Escape) {
            game->switchToMenu();
        }
    }
}

void ProfileSelectionState::update(float deltaTime) {}

void ProfileSelectionState::render(sf::RenderWindow& window) {
    if (backgroundSprite)
    window.draw(*backgroundSprite);
    if (title) window.draw(*title);
    for (auto* box : profileBoxes) window.draw(*box);
    for (auto* text : profileTexts) window.draw(*text);
    if (instructionText) window.draw(*instructionText);
    if (backText) window.draw(*backText);
    if (deleteText) window.draw(*deleteText);
}

void ProfileSelectionState::cleanup() {
    delete title;
    delete instructionText;
    delete backText;
    delete deleteText;
    for (auto* text : profileTexts) delete text;
    for (auto* box : profileBoxes) delete box;
    profileTexts.clear();
    profileBoxes.clear();
}

void ProfileSelectionState::onEnter() {
    std::cout << "Entered Profile Selection State" << std::endl;
    loadProfiles();
    selectedIndex = 0;
    deleteMode = false;
    deleteText->setFillColor(sf::Color(200, 100, 100));
    deleteText->setString("🗑️ PRESS D TO DELETE SELECTED PROFILE");
    updateColors();
}

void ProfileSelectionState::onExit() {
    std::cout << "Exited Profile Selection State" << std::endl;
}
