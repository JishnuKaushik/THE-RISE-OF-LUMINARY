#include "SettingsState.hpp"
#include "Game.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

SettingsState::SettingsState(Game* gameInstance) : game(gameInstance), selectedOption(0) {
    sf::Font& font = game->getMainFont();
    loadSettings();

    bgPanel = new sf::RectangleShape(sf::Vector2f(1280.f, 720.f));
    bgPanel->setPosition(sf::Vector2f(0.f, 0.f));
    bgPanel->setFillColor(sf::Color(8, 6, 24, 245));

    static const float panelY[3] = {200.f, 330.f, 460.f};
    for (int i = 0; i < 3; i++) {
        optionPanels[i] = new sf::RectangleShape(sf::Vector2f(700.f, 90.f));
        optionPanels[i]->setPosition(sf::Vector2f(290.f, panelY[i]));
        optionPanels[i]->setFillColor(sf::Color(18, 14, 44, 200));
        optionPanels[i]->setOutlineColor(sf::Color(160, 120, 35, 160));
        optionPanels[i]->setOutlineThickness(2.f);

        optionTexts[i] = new sf::Text(font, "", 24);
        optionTexts[i]->setPosition(sf::Vector2f(310.f, panelY[i] + 28.f));
        optionTexts[i]->setFillColor(sf::Color(220, 220, 255));
    }

    titleText = new sf::Text(font, "SETTINGS", 48);
    titleText->setPosition(sf::Vector2f(510.f, 80.f));
    titleText->setFillColor(sf::Color(255, 220, 80));

    navText = new sf::Text(font, "UP/DOWN: Select   LEFT/RIGHT: Adjust   ESC/ENTER: Save & Back", 20);
    navText->setPosition(sf::Vector2f(230.f, 648.f));
    navText->setFillColor(sf::Color(150, 150, 180));

    updateOptionTexts();
    std::cout << "SettingsState created" << std::endl;
}

SettingsState::~SettingsState() {
    cleanup();
}

void SettingsState::cleanup() {
    delete bgPanel;
    for (int i = 0; i < 3; i++) {
        delete optionPanels[i];
        delete optionTexts[i];
    }
    delete titleText;
    delete navText;
}

static std::string makeBar(int value, int maxVal = 100) {
    int filled = (value * 20) / maxVal;
    std::string bar = "[";
    for (int i = 0; i < 20; i++) bar += (i < filled ? "=" : " ");
    bar += "] " + std::to_string(value);
    return bar;
}

void SettingsState::updateOptionTexts() {
    optionTexts[0]->setString("Music Volume:   " + makeBar(settings.musicVolume));
    optionTexts[1]->setString("SFX Volume:     " + makeBar(settings.sfxVolume));
    optionTexts[2]->setString("Fullscreen:     " + std::string(settings.fullscreen ? "ON" : "OFF"));

    for (int i = 0; i < 3; i++) {
        bool sel = (i == selectedOption);
        optionPanels[i]->setFillColor(sel ? sf::Color(30, 22, 65, 220) : sf::Color(18, 14, 44, 200));
        optionPanels[i]->setOutlineColor(sel ? sf::Color(255, 220, 80) : sf::Color(160, 120, 35, 160));
        optionPanels[i]->setOutlineThickness(sel ? 3.f : 2.f);
        optionTexts[i]->setFillColor(sel ? sf::Color(255, 235, 160) : sf::Color(220, 220, 255));
    }
}

void SettingsState::handleInput(const sf::Event& event) {
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        switch (kp->code) {
        case sf::Keyboard::Key::Up:
            selectedOption = (selectedOption - 1 + 3) % 3;
            updateOptionTexts();
            break;
        case sf::Keyboard::Key::Down:
            selectedOption = (selectedOption + 1) % 3;
            updateOptionTexts();
            break;
        case sf::Keyboard::Key::Left:
            if      (selectedOption == 0) settings.musicVolume = std::max(0,   settings.musicVolume - 5);
            else if (selectedOption == 1) settings.sfxVolume   = std::max(0,   settings.sfxVolume   - 5);
            else if (selectedOption == 2) settings.fullscreen  = !settings.fullscreen;
            updateOptionTexts();
            break;
        case sf::Keyboard::Key::Right:
            if      (selectedOption == 0) settings.musicVolume = std::min(100, settings.musicVolume + 5);
            else if (selectedOption == 1) settings.sfxVolume   = std::min(100, settings.sfxVolume   + 5);
            else if (selectedOption == 2) settings.fullscreen  = !settings.fullscreen;
            updateOptionTexts();
            break;
        case sf::Keyboard::Key::Enter:
        case sf::Keyboard::Key::Escape:
            saveSettings();
            game->switchToMenu();
            break;
        default: break;
        }
    }
    // Mouse clicks on option panels
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f mp(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));
            for (int i = 0; i < 3; i++) {
                if (optionPanels[i]->getGlobalBounds().contains(mp)) {
                    selectedOption = i;
                    // Toggle bool or nudge volume with a click
                    if (i == 2) { settings.fullscreen = !settings.fullscreen; }
                    updateOptionTexts();
                    break;
                }
            }
        }
    }
}

void SettingsState::update(float /*deltaTime*/) {}

void SettingsState::render(sf::RenderWindow& window) {
    window.draw(*bgPanel);
    for (int i = 0; i < 3; i++) {
        window.draw(*optionPanels[i]);
        window.draw(*optionTexts[i]);
    }
    window.draw(*titleText);
    window.draw(*navText);
}

void SettingsState::onEnter() {
    std::cout << "Entered Settings State" << std::endl;
    selectedOption = 0;
    loadSettings();
    updateOptionTexts();
}

void SettingsState::onExit() {
    std::cout << "Exited Settings State" << std::endl;
    saveSettings();
}

void SettingsState::saveSettings() {
    std::filesystem::create_directory("saves");
    std::ofstream file("saves/settings.json");
    if (!file.is_open()) return;
    file << "{\n"
         << "  \"musicVolume\": " << settings.musicVolume << ",\n"
         << "  \"sfxVolume\": "   << settings.sfxVolume   << ",\n"
         << "  \"fullscreen\": "  << (settings.fullscreen ? "true" : "false") << "\n"
         << "}\n";
    std::cout << "Settings saved." << std::endl;
}

void SettingsState::loadSettings() {
    std::ifstream file("saves/settings.json");
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("\"musicVolume\"") != std::string::npos) {
            size_t c = line.find(':');
            settings.musicVolume = std::stoi(line.substr(c + 1));
        } else if (line.find("\"sfxVolume\"") != std::string::npos) {
            size_t c = line.find(':');
            settings.sfxVolume = std::stoi(line.substr(c + 1));
        } else if (line.find("\"fullscreen\"") != std::string::npos) {
            settings.fullscreen = (line.find("true") != std::string::npos);
        }
    }
}
