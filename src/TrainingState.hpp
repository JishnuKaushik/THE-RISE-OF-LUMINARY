#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>
#include "BattleSystem.hpp"
#include "ScreenShake.hpp"
#include "ParticleSystem.hpp"

struct Question {
    int id;
    std::string subject;
    std::string questionText;
    std::vector<std::string> options;
    int correctIndex;
    std::string explanation;
    std::string funFact;
    std::string hint;
};

class Game;

class TrainingState {
private:
    Game* game;
    BattleSystem battleSystem;
    ScreenShake screenShake;
    ParticleSystem particles;
    sf::Texture backgroundTexture;
    std::optional<sf::Sprite> backgroundSprite;

    // Questions
    std::vector<Question> questions;
    int  currentQuestionIndex;
    int  score;
    int  streak;
    bool answered;
    int  selectedOption;
    bool hintUsed;
    bool inBattle;

    // HP lerp animation
    float displayPlayerHP, targetPlayerHP, maxPlayerHP;
    float displayEnemyHP,  targetEnemyHP,  maxEnemyHP;
    float animTime;

    // ── Core UI ───────────────────────────────────────────────────────────────
    sf::Text* questionText;
    std::vector<sf::Text*> optionTexts;
    std::vector<sf::RectangleShape*> optionBoxes;

    sf::Text* scoreText;
    sf::Text* streakText;
    sf::Text* progressText;
    sf::Text* feedbackText;
    sf::Text* nextButtonText;
    sf::Text* backButtonText;
    sf::Text* hintButtonText;
    sf::Text* masteryText;

    // ── Battle HUD ────────────────────────────────────────────────────────────
    sf::Text* playerHealthText;
    sf::Text* playerStatText;
    sf::Text* enemyNameText;
    sf::Text* enemyHealthText;
    sf::Text* enemyElementText;
    sf::Text* battleLogText;

    sf::RectangleShape* playerHealthBar;
    sf::RectangleShape* playerHealthBarBg;
    sf::RectangleShape* enemyHealthBar;
    sf::RectangleShape* enemyHealthBarBg;

    // ── Panels ────────────────────────────────────────────────────────────────
    sf::RectangleShape* questionBox;
    sf::RectangleShape* feedbackBox;
    sf::RectangleShape* scorePanel;      // centre score/streak panel
    sf::RectangleShape* streakPanel;     // kept for cleanup; not rendered
    sf::RectangleShape* masteryBar;
    sf::RectangleShape* masteryBarFill;
    sf::RectangleShape* playerHUDPanel;
    sf::RectangleShape* enemyHUDPanel;
    sf::RectangleShape* battleLogBg;
    sf::RectangleShape* bottomDivider;

    // ── Colors ────────────────────────────────────────────────────────────────
    sf::Color correctColor;
    sf::Color wrongColor;
    sf::Color normalColor;
    sf::Color selectedColor;
    sf::Color hintColor;
    sf::Color panelColor;

    void loadQuestions();
    void displayQuestion();
    void checkAnswer(int optionIndex);
    void nextQuestion();
    void showHint();
    void saveProgress();
    void updateUI();
    void updateBattleUI();
    void cleanup();
    void startBattle();

public:
    TrainingState(Game* gameInstance);
    ~TrainingState();

    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render(sf::RenderWindow& window);

    void onEnter();
    void onExit();
};
