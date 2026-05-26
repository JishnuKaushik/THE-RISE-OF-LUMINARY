#include "TrainingStoryState.hpp"
#include "Game.hpp"
#include <iostream>

TrainingStoryState::TrainingStoryState(Game* gameInstance) : game(gameInstance), currentChapterIndex(0), currentPart(0), waitingForSubject(true), selectedSubjectIndex(0) {
    sf::Font& font = game->getMainFont();
    
    subjects = {"Science", "Mathematics", "History", "Literature", "Arts"};
    
    titleText = new sf::Text(font, "SELECT A SUBJECT", 40);
    titleText->setFillColor(sf::Color::Yellow);
    titleText->setPosition(sf::Vector2f(500, 80));
    
    contentText = new sf::Text(font, "", 22);
    contentText->setFillColor(sf::Color::White);
    contentText->setPosition(sf::Vector2f(100, 150));
    
    continueText = new sf::Text(font, "PRESS ENTER TO CONTINUE", 20);
    continueText->setFillColor(sf::Color(150, 150, 150));
    continueText->setPosition(sf::Vector2f(500, 650));
    
    backText = new sf::Text(font, "PRESS ESC TO GO BACK", 18);
    backText->setFillColor(sf::Color(150, 150, 150));
    backText->setPosition(sf::Vector2f(550, 680));
    
    for (int i = 0; i < 5; i++) {
        sf::RectangleShape* box = new sf::RectangleShape(sf::Vector2f(300, 50));
        box->setPosition(sf::Vector2f(490, 180 + i * 70));
        box->setFillColor(sf::Color(40, 30, 60));
        box->setOutlineColor(sf::Color(150, 100, 50));
        box->setOutlineThickness(2);
        subjectBoxes.push_back(box);
        
        sf::Text* text = new sf::Text(font, subjects[i], 24);
        text->setFillColor(sf::Color::White);
        text->setPosition(sf::Vector2f(520, 190 + i * 70));
        subjectTexts.push_back(text);
    }
    
    updateSubjectColors();
    std::cout << "Story Mode created - Stories coming soon!" << std::endl;
}

TrainingStoryState::~TrainingStoryState() {
    cleanup();
}

void TrainingStoryState::cleanup() {
    delete titleText;
    delete contentText;
    delete continueText;
    delete backText;
    for (auto* box : subjectBoxes) delete box;
    for (auto* text : subjectTexts) delete text;
    subjectBoxes.clear();
    subjectTexts.clear();
}

void TrainingStoryState::updateSubjectColors() {
    for (size_t i = 0; i < subjectBoxes.size(); i++) {
        if (i == selectedSubjectIndex) {
            subjectBoxes[i]->setOutlineColor(sf::Color::Yellow);
            subjectBoxes[i]->setOutlineThickness(4);
            subjectTexts[i]->setFillColor(sf::Color::Yellow);
        } else {
            subjectBoxes[i]->setOutlineColor(sf::Color(150, 100, 50));
            subjectBoxes[i]->setOutlineThickness(2);
            subjectTexts[i]->setFillColor(sf::Color::White);
        }
    }
}

void TrainingStoryState::loadStoryForSubject(const std::string& subject) {
    std::cout << "Loading stories for: " << subject << std::endl;
}

void TrainingStoryState::displayCurrentPart() {
    if (waitingForSubject) {
        titleText->setString("SELECT A SUBJECT");
    } else {
        titleText->setString("STORY MODE");
        contentText->setString("Educational stories with concepts\nand quick checks will be available soon!\n\nSelect a subject from the menu to begin.");
    }
}

void TrainingStoryState::nextPart() {
    // Placeholder for story navigation
}

void TrainingStoryState::handleInput(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (waitingForSubject) {
            switch (keyPressed->code) {
                case sf::Keyboard::Key::Up:
                    selectedSubjectIndex = (selectedSubjectIndex - 1 + 5) % 5;
                    updateSubjectColors();
                    break;
                case sf::Keyboard::Key::Down:
                    selectedSubjectIndex = (selectedSubjectIndex + 1) % 5;
                    updateSubjectColors();
                    break;
                case sf::Keyboard::Key::Enter:
                    game->selectedSubject = subjects[selectedSubjectIndex];
                    waitingForSubject = false;
                    loadStoryForSubject(game->selectedSubject);
                    displayCurrentPart();
                    break;
                case sf::Keyboard::Key::Escape:
                    game->switchToMenu();
                    break;
                default:
                    break;
            }
        } else {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                waitingForSubject = true;
                displayCurrentPart();
            }
            if (keyPressed->code == sf::Keyboard::Key::Enter) {
                // Future: Navigate through story chapters
            }
        }
    }
}

void TrainingStoryState::update(float deltaTime) {}

void TrainingStoryState::render(sf::RenderWindow& window) {
    window.clear(sf::Color(20, 15, 40));
    
    if (waitingForSubject) {
        window.draw(*titleText);
        for (auto* box : subjectBoxes) window.draw(*box);
        for (auto* text : subjectTexts) window.draw(*text);
        window.draw(*backText);
    } else {
        window.draw(*titleText);
        window.draw(*contentText);
        window.draw(*continueText);
        window.draw(*backText);
    }
}

void TrainingStoryState::onEnter() {
    std::cout << "Entered Story Mode" << std::endl;
    waitingForSubject = true;
    selectedSubjectIndex = 0;
    updateSubjectColors();
    displayCurrentPart();
}

void TrainingStoryState::onExit() {
    std::cout << "Exited Story Mode" << std::endl;
}
