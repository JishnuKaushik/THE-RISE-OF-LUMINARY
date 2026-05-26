#include "TrainingState.hpp"
#include "Game.hpp"
#include "SaveManager.hpp"
#include "Character.hpp"
#include <iostream>
#include <random>
#include <fstream>
#include <algorithm>

// ─── Minimal JSON helpers (internal to this file) ─────────────────────────────
//
// These handle the exact format used by the question JSON files:
//   { "id": N, "question": "...", "options": ["A","B","C","D"],
//     "correct": "A|B|C|D", "explanation": "...", "funFact": "..." }
//
// \uXXXX sequences are decoded to UTF-8 so SFML renders π, ², √, etc. correctly.

static std::pair<std::string, size_t> readJsonStr(const std::string& s, size_t p) {
    // p starts at the character AFTER the opening '"'.
    // Returns {decoded_string, position_of_closing_'"'}.
    std::string out;
    while (p < s.size()) {
        if (s[p] == '"') return {out, p};
        if (s[p] == '\\' && p + 1 < s.size()) {
            char esc = s[p + 1];
            if      (esc == '"')  { out += '"';  p += 2; }
            else if (esc == '\\') { out += '\\'; p += 2; }
            else if (esc == 'n')  { out += '\n'; p += 2; }
            else if (esc == 't')  { out += '\t'; p += 2; }
            else if (esc == 'u' && p + 5 < s.size()) {
                // Decode \uXXXX → UTF-8
                std::string hex = s.substr(p + 2, 4);
                try {
                    unsigned int cp = std::stoul(hex, nullptr, 16);
                    if (cp < 0x80) {
                        out += static_cast<char>(cp);
                    } else if (cp < 0x800) {
                        out += static_cast<char>(0xC0 | (cp >> 6));
                        out += static_cast<char>(0x80 | (cp & 0x3F));
                    } else {
                        out += static_cast<char>(0xE0 | (cp >> 12));
                        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                        out += static_cast<char>(0x80 | (cp & 0x3F));
                    }
                } catch (...) { out += '?'; }
                p += 6;
            } else {
                out += s[p++];
            }
        } else {
            out += s[p++];
        }
    }
    return {out, p};
}

// Extract the string value of a key from a flat JSON object block.
static std::string jsStr(const std::string& obj, const std::string& key) {
    std::string sk = "\"" + key + "\"";
    size_t kp = obj.find(sk);
    if (kp == std::string::npos) return "";
    size_t cp = obj.find(':', kp + sk.size());
    if (cp == std::string::npos) return "";
    size_t q = obj.find('"', cp + 1);
    if (q == std::string::npos) return "";
    return readJsonStr(obj, q + 1).first;
}

// Extract an array of strings for a key from a flat JSON object block.
static std::vector<std::string> jsArr(const std::string& obj, const std::string& key) {
    std::vector<std::string> results;
    std::string sk = "\"" + key + "\"";
    size_t kp = obj.find(sk);
    if (kp == std::string::npos) return results;
    size_t as = obj.find('[', kp + sk.size());
    if (as == std::string::npos) return results;
    size_t ae = obj.find(']', as + 1);
    if (ae == std::string::npos) return results;

    size_t pos = as + 1;
    while (pos < ae) {
        size_t q = obj.find('"', pos);
        if (q == std::string::npos || q >= ae) break;
        auto [val, endPos] = readJsonStr(obj, q + 1);
        results.push_back(val);
        pos = endPos + 1; // advance past closing '"'
    }
    return results;
}

// "A" → 0, "B" → 1, "C" → 2, "D" → 3
static int letterToIdx(const std::string& letter) {
    if (letter.empty()) return 0;
    switch (letter[0]) {
        case 'A': return 0;
        case 'B': return 1;
        case 'C': return 2;
        case 'D': return 3;
        default:  return 0;
    }
}

// Maps the gradeLevel string (as stored in PlayerData) to the folder/prefix pair.
// "Middle School" → "Class_MiddleSchool/MiddleSchool_"
static std::string gradeToFolderPrefix(const std::string& grade) {
    if (grade == "Preschool")     return "Class_Preschool/Preschool_";
    if (grade == "Elementary")    return "Class_Elementary/Elementary_";
    if (grade == "Middle School") return "Class_MiddleSchool/MiddleSchool_";
    if (grade == "High School")   return "Class_HighSchool/HighSchool_";
    if (grade == "College Prep")  return "Class_CollegePrep/CollegePrep_";
    return "";
}

// Maps the subject display name to the filename segment.
// "Mathematics" → "Math"  (all others are identical to the filename segment)
static std::string subjectToFilename(const std::string& subject) {
    if (subject == "Mathematics") return "Math";
    return subject;
}

// ─────────────────────────────────────────────────────────────────────────────

TrainingState::TrainingState(Game* gameInstance) : game(gameInstance),
    currentQuestionIndex(0), score(0), streak(0), answered(false),
    selectedOption(-1), hintUsed(false), inBattle(true) {

    correctColor = sf::Color(0, 200, 0);
    wrongColor   = sf::Color(200, 0, 0);
    normalColor  = sf::Color(220, 220, 220);
    selectedColor = sf::Color(255, 200, 0);
    hintColor    = sf::Color(100, 150, 255);
    panelColor   = sf::Color(40, 30, 60);

    if (backgroundTexture.loadFromFile("assets/images/battlefield.png")) {
        backgroundSprite.emplace(backgroundTexture);
        backgroundSprite->setScale(sf::Vector2f(
            1280.0f / backgroundTexture.getSize().x,
            720.0f  / backgroundTexture.getSize().y
        ));
    }

    sf::Font& font = game->getMainFont();

    optionBoxes.clear();
    optionTexts.clear();

    // Battle UI
    playerHealthText = new sf::Text(font, "LUMINARY: 100/100", 24);
    playerHealthText->setPosition(sf::Vector2f(40, 620));
    playerHealthText->setFillColor(sf::Color::White);

    playerHealthBar = new sf::RectangleShape(sf::Vector2f(250, 20));
    playerHealthBar->setPosition(sf::Vector2f(40, 655));
    playerHealthBar->setFillColor(sf::Color::Green);
    playerHealthBar->setOutlineColor(sf::Color::White);
    playerHealthBar->setOutlineThickness(2);

    enemyNameText = new sf::Text(font, "VS FIRE DRAGON", 28);
    enemyNameText->setPosition(sf::Vector2f(470, 35));
    enemyNameText->setFillColor(sf::Color::Yellow);

    enemyHealthText = new sf::Text(font, "FIRE DRAGON: 150/150", 24);
    enemyHealthText->setPosition(sf::Vector2f(500, 75));
    enemyHealthText->setFillColor(sf::Color::White);

    enemyHealthBar = new sf::RectangleShape(sf::Vector2f(250, 20));
    enemyHealthBar->setPosition(sf::Vector2f(470, 110));
    enemyHealthBar->setFillColor(sf::Color::Red);
    enemyHealthBar->setOutlineColor(sf::Color::White);
    enemyHealthBar->setOutlineThickness(2);

    battleLogText = new sf::Text(font, "Press 1-4 to answer!", 20);
    battleLogText->setPosition(sf::Vector2f(430, 150));
    battleLogText->setFillColor(sf::Color(220, 220, 100));

    // Question box
    questionBox = new sf::RectangleShape(sf::Vector2f(720, 82));
    questionBox->setPosition(sf::Vector2f(390, 240));
    questionBox->setFillColor(sf::Color(10, 10, 20, 190));
    questionBox->setOutlineColor(sf::Color(255, 220, 120, 180));
    questionBox->setOutlineThickness(2);

    questionText = new sf::Text(font, "", 22);
    questionText->setPosition(sf::Vector2f(420, 252));
    questionText->setFillColor(sf::Color::White);

    // Answer options
    for (int i = 0; i < 4; i++) {
        sf::RectangleShape* box = new sf::RectangleShape(sf::Vector2f(440, 42));
        box->setPosition(sf::Vector2f(420, 360 + i * 55));
        box->setFillColor(sf::Color(10, 10, 20, 170));
        box->setOutlineColor(sf::Color(255, 220, 120, 120));
        box->setOutlineThickness(1);
        optionBoxes.push_back(box);

        sf::Text* option = new sf::Text(font, "", 18);
        option->setPosition(sf::Vector2f(440, 368 + i * 55));
        option->setFillColor(normalColor);
        optionTexts.push_back(option);
    }

    // Score panel
    scorePanel = new sf::RectangleShape(sf::Vector2f(170, 60));
    scorePanel->setPosition(sf::Vector2f(1080, 25));
    scorePanel->setFillColor(sf::Color(10, 10, 20, 170));
    scorePanel->setOutlineColor(sf::Color(255, 220, 120, 90));
    scorePanel->setOutlineThickness(1);

    scoreText = new sf::Text(font, "SCORE\n0", 18);
    scoreText->setPosition(sf::Vector2f(1110, 35));
    scoreText->setFillColor(sf::Color(255, 235, 160));

    // Streak panel
    streakPanel = new sf::RectangleShape(sf::Vector2f(170, 60));
    streakPanel->setPosition(sf::Vector2f(1080, 95));
    streakPanel->setFillColor(sf::Color(10, 10, 20, 170));
    streakPanel->setOutlineColor(sf::Color(255, 220, 120, 90));
    streakPanel->setOutlineThickness(1);

    streakText = new sf::Text(font, "STREAK\n0", 18);
    streakText->setPosition(sf::Vector2f(1110, 105));
    streakText->setFillColor(sf::Color(180, 220, 255));

    progressText = new sf::Text(font, "Question 1/10", 18);
    progressText->setPosition(sf::Vector2f(1080, 200));
    progressText->setFillColor(sf::Color(200, 200, 200));

    masteryBar = new sf::RectangleShape(sf::Vector2f(220, 14));
    masteryBar->setPosition(sf::Vector2f(1030, 675));
    masteryBar->setFillColor(sf::Color(15, 15, 25, 170));
    masteryBar->setOutlineColor(sf::Color(255, 220, 120, 80));
    masteryBar->setOutlineThickness(1);

    masteryBarFill = new sf::RectangleShape(sf::Vector2f(0, 14));
    masteryBarFill->setPosition(sf::Vector2f(1030, 675));
    masteryBarFill->setFillColor(sf::Color(120, 255, 170));

    masteryText = new sf::Text(font, "Mastery: 0%", 14);
    masteryText->setPosition(sf::Vector2f(1080, 695));
    masteryText->setFillColor(sf::Color(220, 220, 220));

    // Feedback box
    feedbackBox = new sf::RectangleShape(sf::Vector2f(720, 60));
    feedbackBox->setPosition(sf::Vector2f(390, 600));
    feedbackBox->setFillColor(sf::Color(10, 10, 20, 170));
    feedbackBox->setOutlineColor(sf::Color(255, 220, 120, 100));
    feedbackBox->setOutlineThickness(1);

    feedbackText = new sf::Text(font, "", 16);
    feedbackText->setPosition(sf::Vector2f(410, 615));
    feedbackText->setFillColor(sf::Color::White);

    // Buttons
    nextButtonText = new sf::Text(font, "NEXT (ENTER)", 18);
    nextButtonText->setPosition(sf::Vector2f(640, 675));
    nextButtonText->setFillColor(sf::Color(150, 150, 150));

    backButtonText = new sf::Text(font, "MENU (ESC)", 16);
    backButtonText->setPosition(sf::Vector2f(35, 685));
    backButtonText->setFillColor(sf::Color(150, 150, 150));

    hintButtonText = new sf::Text(font, "HINT (-2) - H", 16);
    hintButtonText->setPosition(sf::Vector2f(1030, 640));
    hintButtonText->setFillColor(hintColor);

    // Questions are NOT loaded here — grade/subject aren't set at construction time.
    // onEnter() calls loadQuestions() after the player has registered or loaded a profile.
    displayQuestion();
    std::cout << "TrainingState created" << std::endl;
}

void TrainingState::loadQuestions() {
    questions.clear();

    std::string grade   = game->playerData.gradeLevel;
    // Prefer game->selectedSubject (live selection); fall back to saved value.
    std::string subject = !game->selectedSubject.empty()
                          ? game->selectedSubject
                          : game->playerData.selectedSubject;

    if (grade.empty() || subject.empty()) {
        std::cerr << "[TrainingState] Grade or subject not set — no questions loaded.\n";
        return;
    }

    std::string folderPrefix = gradeToFolderPrefix(grade);
    if (folderPrefix.empty()) {
        std::cerr << "[TrainingState] Unrecognised grade level: '" << grade << "'\n";
        return;
    }

    std::string path = "data/questions/" + folderPrefix + subjectToFilename(subject) + ".json";
    std::cout << "[TrainingState] Loading: " << path << "\n";

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[TrainingState] Cannot open: " << path << "\n";
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    // Locate the "questions" array
    size_t qkey = content.find("\"questions\"");
    if (qkey == std::string::npos) {
        std::cerr << "[TrainingState] No 'questions' key in " << path << "\n";
        return;
    }
    size_t arrStart = content.find('[', qkey);
    if (arrStart == std::string::npos) return;

    // Walk every { ... } object inside the array
    size_t pos = arrStart + 1;
    while (pos < content.size()) {
        size_t objStart = content.find('{', pos);
        if (objStart == std::string::npos) break;

        // Match braces to find the object boundary
        int depth = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < content.size() && depth > 0) {
            if      (content[objEnd] == '{') ++depth;
            else if (content[objEnd] == '}') --depth;
            ++objEnd;
        }

        std::string obj = content.substr(objStart, objEnd - objStart);

        Question q;
        q.questionText = jsStr(obj, "question");
        q.options      = jsArr(obj, "options");
        q.correctIndex = letterToIdx(jsStr(obj, "correct"));
        q.explanation  = jsStr(obj, "explanation");
        q.funFact      = jsStr(obj, "funFact");
        q.hint         = jsStr(obj, "hint"); // empty string when the key is absent
        q.subject      = subject;
        q.id           = static_cast<int>(questions.size() + 1);

        if (!q.questionText.empty() && q.options.size() == 4) {
            questions.push_back(std::move(q));
        } else {
            std::cerr << "[TrainingState] Skipping malformed question at index "
                      << questions.size() << "\n";
        }

        pos = objEnd;
    }

    // Shuffle so each session presents questions in a different order
    std::mt19937 rng(std::random_device{}());
    std::shuffle(questions.begin(), questions.end(), rng);

    std::cout << "[TrainingState] Loaded " << questions.size()
              << " questions (" << grade << " / " << subject << ")\n";
}

void TrainingState::displayQuestion() {
    if (questions.empty()) {
        questionText->setString("No questions loaded for this subject/grade.\nPress ESC to return to the menu.");
        for (int i = 0; i < 4; i++) optionTexts[i]->setString("");
        progressText->setString("0/0");
        return;
    }
    if (currentQuestionIndex >= (int)questions.size()) return;

    Question& q = questions[currentQuestionIndex];
    questionText->setString(q.questionText);

    for (int i = 0; i < 4; i++) {
        optionTexts[i]->setString(std::string(1, char(65 + i)) + ". " + q.options[i]);
        optionTexts[i]->setFillColor(normalColor);
        optionBoxes[i]->setOutlineColor(sf::Color(255, 220, 120, 120));
        optionBoxes[i]->setFillColor(sf::Color(10, 10, 20, 170));
    }

    progressText->setString("Question " + std::to_string(currentQuestionIndex + 1)
                            + "/" + std::to_string(questions.size()));

    float masteryPercent = ((float)score / ((currentQuestionIndex + 1) * 15)) * 100.f;
    if (masteryPercent > 100.f) masteryPercent = 100.f;
    masteryBarFill->setSize(sf::Vector2f(250.f * (masteryPercent / 100.f), 20.f));
    masteryText->setString("Mastery: " + std::to_string((int)masteryPercent) + "%");

    feedbackText->setString("");
    feedbackBox->setFillColor(sf::Color(30, 20, 50));
    answered     = false;
    selectedOption = -1;
    hintUsed     = false;
    updateUI();
}

void TrainingState::checkAnswer(int optionIndex) {
    if (answered) return;

    answered = true;
    selectedOption = optionIndex;
    Question& q = questions[currentQuestionIndex];

    // Track stats for the save file
    game->playerData.totalQuestionsAnswered++;

    optionBoxes[optionIndex]->setOutlineColor(sf::Color(255, 220, 120));
    optionBoxes[optionIndex]->setOutlineThickness(3);

    if (optionIndex == q.correctIndex) {
        game->playerData.correctAnswers++;
        int points = 10 + (streak * 2);
        score += points;
        streak++;

        screenShake.start(0.2f, 6.0f);
        particles.spawnSparkles(optionBoxes[optionIndex]->getPosition());

        battleLogText->setString("CORRECT! +" + std::to_string(points) + " points!");
        battleLogText->setFillColor(sf::Color(120, 255, 170));

        feedbackText->setString("CORRECT! +" + std::to_string(points) + " points!\n" + q.explanation);
        feedbackBox->setFillColor(sf::Color(15, 45, 25, 190));
        optionBoxes[optionIndex]->setFillColor(sf::Color(20, 60, 35, 180));

        std::cout << "Correct! Streak: " << streak << " Score: " << score << std::endl;
    } else {
        streak = 0;

        screenShake.start(0.2f, 4.0f);
        particles.spawnSmoke(optionBoxes[optionIndex]->getPosition());

        battleLogText->setString("WRONG! Enemy attacks!");
        battleLogText->setFillColor(sf::Color(255, 140, 140));

        feedbackText->setString("INCORRECT!\nCorrect: " + q.options[q.correctIndex] + "\n" + q.explanation);
        feedbackBox->setFillColor(sf::Color(55, 20, 20, 190));
        optionBoxes[optionIndex]->setFillColor(sf::Color(75, 25, 25, 180));
        optionBoxes[q.correctIndex]->setOutlineColor(sf::Color(120, 255, 170));
        optionBoxes[q.correctIndex]->setOutlineThickness(3);

        std::cout << "Wrong! Streak reset. Score: " << score << std::endl;
    }

    updateUI();
    saveProgress();
}

void TrainingState::nextQuestion() {
    if (!answered) return;

    currentQuestionIndex++;

    if (currentQuestionIndex >= (int)questions.size()) {
        feedbackText->setString("CHAPTER COMPLETE!\nFinal Score: " + std::to_string(score));
        nextButtonText->setString("PRESS ENTER FOR MENU");
        particles.spawnSparkles(sf::Vector2f(640, 360));
        battleLogText->setString("VICTORY!");
    } else {
        displayQuestion();
        battleLogText->setString("Next question! Press 1-4 to answer!");
        battleLogText->setFillColor(sf::Color(220, 220, 100));
    }
}

void TrainingState::showHint() {
    if (answered || hintUsed) return;

    hintUsed = true;
    score -= 2;
    if (score < 0) score = 0;

    Question& q = questions[currentQuestionIndex];
    // JSON files don't include hints yet — fall back to a generic prompt
    std::string hintText = q.hint.empty()
                           ? "Think carefully — eliminate obviously wrong choices first."
                           : q.hint;
    feedbackText->setString("HINT: " + hintText + " (-2 points)");
    feedbackBox->setFillColor(sf::Color(50, 40, 100));
    updateUI();
    saveProgress();
}

void TrainingState::startBattle() {
    inBattle = true;
    battleLogText->setString("BATTLE STARTED! Answer to attack!");
}

void TrainingState::updateUI() {
    scoreText->setString("SCORE\n" + std::to_string(score));
    streakText->setString("STREAK\n" + std::to_string(streak));
}

void TrainingState::updateBattleUI() {}

void TrainingState::saveProgress() {
    if (!game->hasLoadedProfile && game->playerData.username.empty()) return;
    game->playerData.totalScore = score;
    game->playerData.bestStreak = std::max(game->playerData.bestStreak, streak);
    SaveManager::savePlayer(game->playerData);
}

void TrainingState::handleInput(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (!answered && !questions.empty()) {
            switch (keyPressed->code) {
                case sf::Keyboard::Key::Num1: checkAnswer(0); break;
                case sf::Keyboard::Key::Num2: checkAnswer(1); break;
                case sf::Keyboard::Key::Num3: checkAnswer(2); break;
                case sf::Keyboard::Key::Num4: checkAnswer(3); break;
                default: break;
            }
        }

        if (keyPressed->code == sf::Keyboard::Key::Enter && answered) {
            if (currentQuestionIndex >= (int)questions.size()) {
                game->switchToMenu();
            } else {
                nextQuestion();
            }
        }

        if (keyPressed->code == sf::Keyboard::Key::H && !answered && !hintUsed) {
            showHint();
        }

        if (keyPressed->code == sf::Keyboard::Key::Escape) {
            game->switchToMenu();
        }
    }
}

void TrainingState::update(float deltaTime) {
    screenShake.update(deltaTime);
    particles.update(deltaTime);
}

void TrainingState::render(sf::RenderWindow& window) {
    sf::View view = window.getView();
    sf::Vector2f shakeOffset = screenShake.getOffset();
    view.setCenter(sf::Vector2f(640 + shakeOffset.x, 360 + shakeOffset.y));
    window.setView(view);

    if (backgroundSprite.has_value()) window.draw(*backgroundSprite);
    if (playerHealthText) window.draw(*playerHealthText);
    if (playerHealthBar)  window.draw(*playerHealthBar);
    if (enemyNameText)    window.draw(*enemyNameText);
    if (enemyHealthText)  window.draw(*enemyHealthText);
    if (enemyHealthBar)   window.draw(*enemyHealthBar);
    if (battleLogText)    window.draw(*battleLogText);
    if (questionBox)      window.draw(*questionBox);
    if (questionText)     window.draw(*questionText);
    for (auto* box  : optionBoxes)  window.draw(*box);
    for (auto* text : optionTexts)  window.draw(*text);
    if (scorePanel)       window.draw(*scorePanel);
    if (scoreText)        window.draw(*scoreText);
    if (streakPanel)      window.draw(*streakPanel);
    if (streakText)       window.draw(*streakText);
    if (progressText)     window.draw(*progressText);
    if (masteryBar)       window.draw(*masteryBar);
    if (masteryBarFill)   window.draw(*masteryBarFill);
    if (masteryText)      window.draw(*masteryText);
    if (feedbackBox)      window.draw(*feedbackBox);
    if (feedbackText)     window.draw(*feedbackText);
    if (nextButtonText)   window.draw(*nextButtonText);
    if (backButtonText)   window.draw(*backButtonText);
    if (hintButtonText)   window.draw(*hintButtonText);

    particles.render(window);

    window.setView(window.getDefaultView());
}

void TrainingState::cleanup() {
    delete playerHealthText;
    delete playerHealthBar;
    delete enemyNameText;
    delete enemyHealthText;
    delete enemyHealthBar;
    delete battleLogText;
    delete questionBox;
    delete questionText;
    for (auto* box  : optionBoxes)  delete box;
    for (auto* text : optionTexts)  delete text;
    delete scorePanel;
    delete scoreText;
    delete streakPanel;
    delete streakText;
    delete progressText;
    delete masteryBar;
    delete masteryBarFill;
    delete masteryText;
    delete feedbackBox;
    delete feedbackText;
    delete nextButtonText;
    delete backButtonText;
    delete hintButtonText;
    optionBoxes.clear();
    optionTexts.clear();
}

TrainingState::~TrainingState() {
    cleanup();
}

void TrainingState::onEnter() {
    std::cout << "Entered Training State" << std::endl;
    currentQuestionIndex = 0;
    score  = 0;
    streak = 0;
    answered = false;
    inBattle = true;

    // Load AFTER grade/subject are set by registration or profile loading
    loadQuestions();

    startBattle();
    displayQuestion();
}

void TrainingState::onExit() {
    std::cout << "Exited Training State" << std::endl;
    saveProgress();
}
