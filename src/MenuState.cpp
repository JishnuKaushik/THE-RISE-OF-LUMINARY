#include "MenuState.hpp"
#include "Game.hpp"
#include "SaveManager.hpp"
#include <iostream>

MenuState::MenuState(Game* gameInstance) : game(gameInstance), selectedIndex(0) {
    selectedColor = sf::Color(255, 235, 160);
    normalColor = sf::Color(245, 245, 245);
    if (backgroundTexture.loadFromFile("assets/images/menu.png")) {
    backgroundSprite.emplace(backgroundTexture);

    backgroundSprite->setScale(
        sf::Vector2f(
            1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y
        )
    );
}

    // Menu options
    options = {
        "NEW GAME",
        "LOAD PROFILE",
        "TRAINING MODE",
        "STORY MODE",
        "STATISTICS",
        "SETTINGS",
        "EXIT"
    };
    
    sf::Font& font = game->getMainFont();
    
    // Title
    title = new sf::Text(font, "THE RISING OF THE LUMINARY", 56);
    title->setPosition(sf::Vector2f(300, 80));
    title->setFillColor(sf::Color::Yellow);
    
    // Create menu items
    float startY = 250;
    for (size_t i = 0; i < options.size(); i++) {

    Button* button = new Button(
        font,
        options[i],
        20,
        sf::Vector2f(260, 42),
        sf::Vector2f(900, startY + i * 55)
    );

    menuButtons.push_back(button);
}
    
    updateBoxColors();
    updateTextColors();
    
    std::cout << "MenuState created with " << options.size() << " options" << std::endl;
}

MenuState::~MenuState() {
    cleanup();
}

void MenuState::cleanup() {
    delete title;
    for (auto* button : menuButtons) delete button;
    menuButtons.clear();
}

void MenuState::updateBoxColors() {
    for (size_t i = 0; i < menuButtons.size(); i++) {
        menuButtons[i]->setSelected(i == selectedIndex);
    }
}


void MenuState::updateTextColors() {
}

void MenuState::handleInput(const sf::Event& event) {

if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {

    sf::Vector2f mousePos(
        static_cast<float>(mouseMoved->position.x),
        static_cast<float>(mouseMoved->position.y)
    );

    for (size_t i = 0; i < menuButtons.size(); i++) {

        bool hovering = menuButtons[i]->getBounds().contains(mousePos);

        menuButtons[i]->setHovered(hovering);

        if (hovering) {
            selectedIndex = i;
            updateBoxColors();
        }
    }
}
     if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {

    if (mousePressed->button == sf::Mouse::Button::Left) {

        sf::Vector2f mousePos(
            static_cast<float>(mousePressed->position.x),
            static_cast<float>(mousePressed->position.y)
        );

        for (size_t i = 0; i < menuButtons.size(); i++) {

            if (menuButtons[i]->getBounds().contains(mousePos)) {

                selectedIndex = i;

                std::cout << "Mouse clicked: " << options[selectedIndex] << std::endl;

                if (selectedIndex == 0) {
                    game->hasLoadedProfile = false;
                    game->startFade([this]{ game->switchToRegistration(); });
                }
                else if (selectedIndex == 1) {
                    game->startFade([this]{ game->switchToProfileSelection(); });
                }
                else if (selectedIndex == 2) {
                    game->startFade([this]{ game->switchToTrainingHub(); });
                }
                else if (selectedIndex == 3) {
                    game->startFade([this]{ game->switchToStoryMode(); });
                }
                else if (selectedIndex == 4) {
                    game->startFade([this]{ game->switchToStatistics(); });
                }
                else if (selectedIndex == 5) {
                    game->startFade([this]{ game->switchToSettings(); });
                }
                else if (selectedIndex == 6) {
                    game->getWindow().close();
                }

                break;
            }
        }
    }
}

     if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        switch (keyPressed->code) {
            case sf::Keyboard::Key::Up:
                selectedIndex = (selectedIndex - 1 + options.size()) % options.size();
                updateBoxColors();
                updateTextColors();
                std::cout << "Selected: " << options[selectedIndex] << std::endl;
                break;

            case sf::Keyboard::Key::Down:
                selectedIndex = (selectedIndex + 1) % options.size();
                updateBoxColors();
                updateTextColors();
                std::cout << "Selected: " << options[selectedIndex] << std::endl;
                break;

            case sf::Keyboard::Key::Enter:
                std::cout << "Enter pressed on: " << options[selectedIndex] << std::endl;

                if (selectedIndex == 0) {
                    game->hasLoadedProfile = false;
                    game->startFade([this]{ game->switchToRegistration(); });
                }
                else if (selectedIndex == 1) {
                    game->startFade([this]{ game->switchToProfileSelection(); });
                }
                else if (selectedIndex == 2) {
                    game->startFade([this]{ game->switchToTrainingHub(); });
                }
                else if (selectedIndex == 3) {
                    game->startFade([this]{ game->switchToStoryMode(); });
                }
                else if (selectedIndex == 4) {
                    game->startFade([this]{ game->switchToStatistics(); });
                }
                else if (selectedIndex == 5) {
                    game->startFade([this]{ game->switchToSettings(); });
                }
                else if (selectedIndex == 6) {
                    game->getWindow().close();
                }
                break;

            default:
                break;
        }
    }
}

void MenuState::update(float deltaTime) {
    for (auto* button : menuButtons)
        button->update(deltaTime);
}

void MenuState::render(sf::RenderWindow& window) {
    if (backgroundSprite.has_value()) {
    window.draw(*backgroundSprite);
}
    //if (title) window.draw(*title);
    for (auto* button : menuButtons) {
    button->render(window);
}
}

void MenuState::onEnter() {
    std::cout << "Entered Menu State" << std::endl;
    selectedIndex = 0;
    updateBoxColors();
    updateTextColors();
}

void MenuState::onExit() {
    std::cout << "Exited Menu State" << std::endl;
}
