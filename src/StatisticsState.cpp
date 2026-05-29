#include "StatisticsState.hpp"
#include "Game.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

StatisticsState::StatisticsState(Game* gameInstance) : game(gameInstance) {
    sf::Font& font = game->getMainFont();

    bgPanel = new sf::RectangleShape(sf::Vector2f(1280.f, 720.f));
    bgPanel->setPosition(sf::Vector2f(0.f, 0.f));
    bgPanel->setFillColor(sf::Color(8, 6, 24, 245));

    overallPanel = new sf::RectangleShape(sf::Vector2f(580.f, 420.f));
    overallPanel->setPosition(sf::Vector2f(40.f, 140.f));
    overallPanel->setFillColor(sf::Color(18, 14, 44, 200));
    overallPanel->setOutlineColor(sf::Color(160, 120, 35, 180));
    overallPanel->setOutlineThickness(2.f);

    subjectPanel = new sf::RectangleShape(sf::Vector2f(580.f, 420.f));
    subjectPanel->setPosition(sf::Vector2f(660.f, 140.f));
    subjectPanel->setFillColor(sf::Color(18, 14, 44, 200));
    subjectPanel->setOutlineColor(sf::Color(160, 120, 35, 180));
    subjectPanel->setOutlineThickness(2.f);

    titleText = new sf::Text(font, "STATISTICS", 46);
    titleText->setPosition(sf::Vector2f(480.f, 52.f));
    titleText->setFillColor(sf::Color(255, 220, 80));

    overallText = new sf::Text(font, "", 20);
    overallText->setPosition(sf::Vector2f(60.f, 160.f));
    overallText->setFillColor(sf::Color(220, 220, 255));

    subjectText = new sf::Text(font, "", 20);
    subjectText->setPosition(sf::Vector2f(680.f, 160.f));
    subjectText->setFillColor(sf::Color(220, 220, 255));

    navText = new sf::Text(font, "ESC / ENTER: Back to Menu", 20);
    navText->setPosition(sf::Vector2f(460.f, 680.f));
    navText->setFillColor(sf::Color(150, 150, 180));

    std::cout << "StatisticsState created" << std::endl;
}

StatisticsState::~StatisticsState() {
    cleanup();
}

void StatisticsState::cleanup() {
    delete bgPanel;
    delete overallPanel;
    delete subjectPanel;
    delete titleText;
    delete overallText;
    delete subjectText;
    delete navText;
}

void StatisticsState::buildStats() {
    const PlayerData& pd = game->playerData;

    // ── Overall panel ─────────────────────────────────────────────────────────
    float overallAcc = (pd.totalQuestionsAnswered > 0)
        ? (pd.correctAnswers * 100.f / pd.totalQuestionsAnswered)
        : 0.f;

    std::ostringstream ov;
    ov << "PLAYER: " << pd.username << "\n\n"
       << "Grade:    " << pd.gradeLevel << "\n"
       << "Subject:  " << pd.selectedSubject << "\n\n"
       << "Questions:  " << pd.totalQuestionsAnswered << "\n"
       << "Correct:    " << pd.correctAnswers << "\n"
       << std::fixed << std::setprecision(1)
       << "Accuracy:   " << overallAcc << "%\n\n"
       << "Best Streak:    " << pd.bestStreak << "\n"
       << "Login Streak:   " << pd.currentStreak << " days\n\n"
       << "Chapters Done:  " << pd.completedChapters.size() << "\n\n"
       << "--- DAILY QUESTS ---\n"
       << (pd.dailyQuestionsAnswered >= 10 ? "[X]" : "[ ]")
       << " Answer 10 questions (" << pd.dailyQuestionsAnswered << "/10)\n"
       << (pd.dailyChaptersCompleted >= 1 ? "[X]" : "[ ]")
       << " Complete a story chapter (" << pd.dailyChaptersCompleted << "/1)\n"
       << (pd.currentStreak >= 2 ? "[X]" : "[ ]")
       << " 2-day login streak (" << pd.currentStreak << " days)";
    overallText->setString(ov.str());

    // ── Per-subject panel ─────────────────────────────────────────────────────
    static const char* SUBJS[] = {"Mathematics","Science","History","Literature","Arts"};

    std::string strongest, weakest;
    float bestAcc = -1.f, worstAcc = 101.f;

    std::ostringstream sb;
    sb << "PER-SUBJECT ACCURACY\n\n";
    for (const char* s : SUBJS) {
        int att = pd.topicAttempts.count(s) ? pd.topicAttempts.at(s) : 0;
        int cor = pd.topicCorrect.count(s)  ? pd.topicCorrect.at(s)  : 0;
        float acc = (att > 0) ? (cor * 100.f / att) : 0.f;

        std::ostringstream line;
        line << s << ": ";
        if (att == 0) line << "No data";
        else          line << std::fixed << std::setprecision(1) << acc << "% (" << cor << "/" << att << ")";
        sb << line.str() << "\n";

        if (att > 0) {
            if (acc > bestAcc)  { bestAcc  = acc;  strongest = s; }
            if (acc < worstAcc) { worstAcc = acc;  weakest   = s; }
        }
    }

    sb << "\n";
    if (!strongest.empty()) sb << "Strongest: " << strongest << "\n";
    if (!weakest.empty() && weakest != strongest) sb << "Weakest:   " << weakest << "\n";

    subjectText->setString(sb.str());
}

void StatisticsState::handleInput(const sf::Event& event) {
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape ||
            kp->code == sf::Keyboard::Key::Enter) {
            game->switchToMenu();
        }
    }
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left)
            game->switchToMenu();
    }
}

void StatisticsState::update(float /*deltaTime*/) {}

void StatisticsState::render(sf::RenderWindow& window) {
    window.draw(*bgPanel);
    window.draw(*overallPanel);
    window.draw(*subjectPanel);
    window.draw(*titleText);
    window.draw(*overallText);
    window.draw(*subjectText);
    window.draw(*navText);
}

void StatisticsState::onEnter() {
    std::cout << "Entered Statistics State" << std::endl;
    buildStats();
}

void StatisticsState::onExit() {
    std::cout << "Exited Statistics State" << std::endl;
}
