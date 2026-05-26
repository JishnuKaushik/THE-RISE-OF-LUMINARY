#include "RegistrationState.hpp"
#include "Game.hpp"
#include "SaveManager.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>

RegistrationState::RegistrationState(Game* gameInstance) : game(gameInstance), selectedIndex(0), isTypingName(true), isTypingAge(false) {
    selectedColor = sf::Color::Yellow;
    normalColor = sf::Color::White;
    
    subjects = {"Mathematics", "Science", "History", "Literature", "Arts"};
    gradeLevels = {"Preschool (3-5)", "Elementary (6-10)", "Middle School (11-13)", "High School (14-17)", "College Prep (18+)"};
    
    sf::Font& font = game->getMainFont();
    backgroundTexture.emplace();

if (backgroundTexture->loadFromFile("assets/images/registration_bg.png")) {

    backgroundSprite.emplace(*backgroundTexture);

    sf::Vector2u textureSize = backgroundTexture->getSize();

    backgroundSprite->setScale(sf::Vector2f(
        1280.f / textureSize.x,
        720.f / textureSize.y
    ));
}    

    //title = new sf::Text(font, "CREATE YOUR PROFILE", 48);
    //title->setPosition(sf::Vector2f(420, 50));
    //title->setFillColor(sf::Color::Yellow);
    
namePrompt = new sf::Text(font, "Enter your name:", 18);
namePrompt->setPosition(sf::Vector2f(290, 180));
namePrompt->setFillColor(sf::Color(235, 220, 185));

nameInput = new sf::Text(font, "_", 18);
nameInput->setPosition(sf::Vector2f(290, 215));
nameInput->setFillColor(sf::Color(120, 220, 255));

agePrompt = new sf::Text(font, "Enter your age:", 18);
agePrompt->setPosition(sf::Vector2f(290, 255));
agePrompt->setFillColor(sf::Color(235, 220, 185));

ageInput = new sf::Text(font, "", 18);
ageInput->setPosition(sf::Vector2f(290, 290));
ageInput->setFillColor(sf::Color(120, 220, 255));

gradePrompt = new sf::Text(font, "Recommended Grade Level:", 18);
gradePrompt->setPosition(sf::Vector2f(290, 330));
gradePrompt->setFillColor(sf::Color(235, 220, 185));

gradeDisplay = new sf::Text(font, "", 18);
gradeDisplay->setPosition(sf::Vector2f(290, 365));
gradeDisplay->setFillColor(sf::Color(255, 215, 90));

subjectPrompt = new sf::Text(font, "Choose your subject:", 18);
subjectPrompt->setPosition(sf::Vector2f(290, 395));
subjectPrompt->setFillColor(sf::Color(235, 220, 185));

for (size_t i = 0; i < subjects.size(); i++) {

    sf::Text* subjectText = new sf::Text(font, subjects[i], 17);

    subjectText->setPosition(sf::Vector2f(300, 440 + i * 22));

    subjectText->setFillColor(sf::Color(210, 210, 220));

    subjectTexts.push_back(subjectText);
}

instructionText = new sf::Text(
    font,
    "ENTER = Confirm   |   ESC = Back",
    12
);

instructionText->setPosition(sf::Vector2f(110, 640));

instructionText->setFillColor(sf::Color(150, 150, 170));

usernameInputText = "";
ageInputText = "";
// updateTextColors();

std::cout << "RegistrationState created" << std::endl;
}

RegistrationState::~RegistrationState() {
    cleanup();
}

void RegistrationState::cleanup() {
    // delete title;
    delete namePrompt;
    delete nameInput;
    delete agePrompt;
    delete ageInput;
    delete gradePrompt;
    delete gradeDisplay;
    delete subjectPrompt;
    delete instructionText;
    for (auto* text : subjectTexts) delete text;
    subjectTexts.clear();
}

void RegistrationState::updateTextColors() {
    for (size_t i = 0; i < subjectTexts.size(); i++) {
        subjectTexts[i]->setFillColor(i == selectedIndex ? selectedColor : normalColor);
    }
}

void RegistrationState::updateGradeLevel() {
    int age = 0;
    try {
        age = std::stoi(ageInputText);
    } catch (...) {
        gradeDisplay->setString("Enter age first");
        return;
    }
    
    if (age >= 3 && age <= 5) {
        gradeDisplay->setString(gradeLevels[0]);
        game->playerData.gradeLevel = "Preschool";
    } else if (age >= 6 && age <= 10) {
        gradeDisplay->setString(gradeLevels[1]);
        game->playerData.gradeLevel = "Elementary";
    } else if (age >= 11 && age <= 13) {
        gradeDisplay->setString(gradeLevels[2]);
        game->playerData.gradeLevel = "Middle School";
    } else if (age >= 14 && age <= 17) {
        gradeDisplay->setString(gradeLevels[3]);
        game->playerData.gradeLevel = "High School";
    } else if (age >= 18) {
        gradeDisplay->setString(gradeLevels[4]);
        game->playerData.gradeLevel = "College Prep";
    } else {
        gradeDisplay->setString("Invalid age (min 3)");
    }
}

void RegistrationState::saveProfile() {
    game->playerData.username = usernameInputText;
    game->playerData.age = std::stoi(ageInputText);
    game->playerData.selectedSubject = subjects[selectedIndex];
    game->playerData.totalScore = 0;
    game->playerData.bestStreak = 0;
    game->playerData.totalQuestionsAnswered = 0;
    game->playerData.correctAnswers = 0;
    game->playerData.currentStreak = 0;
    game->playerData.dailyMinutes = 0;
    game->playerData.selectedCharacter = 1; // Default character
    
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d");
    game->playerData.lastPlayedDate = ss.str();
    
    SaveManager::savePlayer(game->playerData);
    game->selectedSubject = subjects[selectedIndex];
    std::cout << "Profile saved: " << game->playerData.username << std::endl;
}

void RegistrationState::handleInput(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        int keyCode = static_cast<int>(keyPressed->code);
        
        if (isTypingName) {
            if (keyCode == 58) {
                isTypingName = false;
                isTypingAge = true;
                nameInput->setString(usernameInputText.empty() ? "Player" : usernameInputText);
                if (usernameInputText.empty()) usernameInputText = "Player";
                nameInput->setFillColor(normalColor);
                ageInput->setString("_");
                ageInput->setFillColor(sf::Color::Cyan);
            }
            else if (keyCode == 59) {
                if (!usernameInputText.empty()) {
                    usernameInputText.pop_back();
                    nameInput->setString(usernameInputText + "_");
                }
            }
            return;
        }
        
        if (isTypingAge) {
            if (keyCode == 58) {
                if (!ageInputText.empty()) {
                    isTypingAge = false;
                    ageInput->setString(ageInputText);
                    ageInput->setFillColor(normalColor);
                    updateGradeLevel();
                }
            }
            else if (keyCode == 59) {
                if (!ageInputText.empty()) {
                    ageInputText.pop_back();
                    ageInput->setString(ageInputText + "_");
                }
            }
            else if (keyCode >= 26 && keyCode <= 35) {
                int num = keyCode - 26;
                ageInputText += std::to_string(num);
                ageInput->setString(ageInputText + "_");
            }
            return;
        }
        
        switch (keyPressed->code) {
            case sf::Keyboard::Key::Up:
                selectedIndex = (selectedIndex - 1 + subjectTexts.size()) % subjectTexts.size();
                updateTextColors();
                break;
                
            case sf::Keyboard::Key::Down:
                selectedIndex = (selectedIndex + 1) % subjectTexts.size();
                updateTextColors();
                break;
                
            case sf::Keyboard::Key::Enter:
                saveProfile();
                game->switchToCharacterSelection();  // GO TO CHARACTER SELECTION
                break;
                
            case sf::Keyboard::Key::Escape:
                game->switchToMenu();
                break;
                
            default:
                break;
        }
    }
    
    if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
        if (isTypingName && textEntered->unicode < 128 && textEntered->unicode != 13) {
            char c = static_cast<char>(textEntered->unicode);
            if (isalnum(c) || c == ' ') {
                usernameInputText += c;
                nameInput->setString(usernameInputText + "_");
            }
        }
    }
}

void RegistrationState::update(float deltaTime) {}

void RegistrationState::render(sf::RenderWindow& window) {
    if (backgroundSprite.has_value()) {
    window.draw(*backgroundSprite);
}

   // if (title) window.draw(*title);
    if (namePrompt) window.draw(*namePrompt);
    if (nameInput) window.draw(*nameInput);
    if (agePrompt) window.draw(*agePrompt);
    if (ageInput) window.draw(*ageInput);
    if (gradePrompt) window.draw(*gradePrompt);
    if (gradeDisplay) window.draw(*gradeDisplay);
    if (subjectPrompt) window.draw(*subjectPrompt);
    for (auto* text : subjectTexts) window.draw(*text);
    if (instructionText) window.draw(*instructionText);
}

void RegistrationState::onEnter() {
    std::cout << "Entered Registration State" << std::endl;
    selectedIndex = 0;
    isTypingName = true;
    isTypingAge = false;
    usernameInputText = "";
    ageInputText = "";
    nameInput->setString("_");
    ageInput->setString("");
    nameInput->setFillColor(sf::Color::Cyan);
    ageInput->setFillColor(sf::Color::White);
    updateTextColors();
}

void RegistrationState::onExit() {
    std::cout << "Exited Registration State" << std::endl;
}
