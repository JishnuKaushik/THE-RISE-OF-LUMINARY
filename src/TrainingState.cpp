#include "TrainingState.hpp"
#include "Game.hpp"
#include "SaveManager.hpp"
#include "Character.hpp"
#include <iostream>
#include <random>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath>

// ─── Minimal JSON helpers ─────────────────────────────────────────────────────
static std::pair<std::string, size_t> readJsonStr(const std::string& s, size_t p) {
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
        pos = endPos + 1;
    }
    return results;
}

static int letterToIdx(const std::string& letter) {
    if (letter.empty()) return 0;
    switch (letter[0]) {
        case 'A': return 0; case 'B': return 1;
        case 'C': return 2; case 'D': return 3;
        default:  return 0;
    }
}

static std::string gradeToFolderPrefix(const std::string& grade) {
    if (grade == "Preschool")     return "Class_Preschool/Preschool_";
    if (grade == "Elementary")    return "Class_Elementary/Elementary_";
    if (grade == "Middle School") return "Class_MiddleSchool/MiddleSchool_";
    if (grade == "High School")   return "Class_HighSchool/HighSchool_";
    if (grade == "College Prep")  return "Class_CollegePrep/CollegePrep_";
    return "";
}

static std::string subjectToFilename(const std::string& subject) {
    if (subject == "Mathematics") return "Math";
    return subject;
}

// ─── UI Helpers ───────────────────────────────────────────────────────────────

static float lerpF(float a, float b, float t) {
    return a + (b - a) * t;
}

// Word-wrap: inserts \n to keep each line within maxWidth pixels.
static std::string wrapText(const std::string& text, const sf::Font& font,
                             unsigned int charSize, float maxWidth) {
    std::string result, currentLine, word;
    auto flushWord = [&]() {
        if (word.empty()) return;
        std::string test = currentLine.empty() ? word : currentLine + " " + word;
        sf::Text probe(font, test, charSize);
        if (!currentLine.empty() && probe.getLocalBounds().size.x > maxWidth) {
            if (!result.empty()) result += '\n';
            result += currentLine;
            currentLine = word;
        } else {
            currentLine = test;
        }
        word.clear();
    };
    for (char c : text) {
        if (c == ' ')       { flushWord(); }
        else if (c == '\n') { flushWord(); if (!result.empty()) result += '\n'; result += currentLine; currentLine.clear(); }
        else                { word += c; }
    }
    flushWord();
    if (!currentLine.empty()) { if (!result.empty()) result += '\n'; result += currentLine; }
    return result;
}

// Drop shadow text render (saves/restores fill color).
static void drawShadowText(sf::RenderTarget& target, sf::Text& text, sf::Vector2f pos) {
    sf::Color mainColor = text.getFillColor();
    text.setPosition(pos + sf::Vector2f(2.f, 2.f));
    text.setFillColor(sf::Color(0, 0, 0, 160));
    target.draw(text);
    text.setPosition(pos);
    text.setFillColor(mainColor);
    target.draw(text);
}

// Glassmorphism panel: translucent fill + top-edge highlight + animated gold border + corner accents.
// glowIntensity [0..1] brightens the border for pulsing effects.
static void drawGlassPanel(sf::RenderTarget& target, sf::FloatRect b,
                            float glowIntensity = 0.f,
                            sf::Color bg = sf::Color(15, 12, 40, 145)) {
    // Soft drop shadow
    sf::RectangleShape shadow(b.size + sf::Vector2f(8.f, 8.f));
    shadow.setPosition(b.position + sf::Vector2f(4.f, 5.f));
    shadow.setFillColor(sf::Color(0, 0, 0, 65));
    target.draw(shadow);

    // Main glass fill
    sf::RectangleShape panel(b.size);
    panel.setPosition(b.position);
    panel.setFillColor(bg);
    target.draw(panel);

    // Inner top-edge refraction highlight
    sf::RectangleShape highlight(sf::Vector2f(b.size.x * 0.86f, 1.5f));
    highlight.setPosition(sf::Vector2f(b.position.x + b.size.x * 0.07f, b.position.y + 3.f));
    highlight.setFillColor(sf::Color(210, 190, 255, 30));
    target.draw(highlight);

    // Animated gold border
    auto clamp = [](int v) -> uint8_t { return static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v)); };
    sf::Color borderCol(
        clamp(165 + (int)(glowIntensity * 90)),
        clamp(125 + (int)(glowIntensity * 30)),
        40,
        clamp(150 + (int)(glowIntensity * 105))
    );
    const float thick = 1.5f;
    const float cx = b.position.x, cy = b.position.y;
    const float cw = b.size.x, ch = b.size.y;
    sf::RectangleShape r;
    r.setFillColor(borderCol);
    r.setSize({cw, thick});  r.setPosition({cx, cy});          target.draw(r);
                              r.setPosition({cx, cy+ch-thick}); target.draw(r);
    r.setSize({thick, ch});  r.setPosition({cx, cy});           target.draw(r);
                              r.setPosition({cx+cw-thick, cy}); target.draw(r);

    // Corner accent squares
    const float cs = 7.f;
    r.setSize({cs, cs});
    r.setFillColor(sf::Color(255, 220, 80, clamp(155 + (int)(glowIntensity * 100))));
    r.setPosition({cx,       cy});        target.draw(r);
    r.setPosition({cx+cw-cs, cy});        target.draw(r);
    r.setPosition({cx,       cy+ch-cs});  target.draw(r);
    r.setPosition({cx+cw-cs, cy+ch-cs});  target.draw(r);
}

// Button inner-top shine line.
static void drawButtonShine(sf::RenderTarget& target, sf::FloatRect b) {
    sf::RectangleShape shine(sf::Vector2f(b.size.x * 0.78f, 1.f));
    shine.setPosition(sf::Vector2f(b.position.x + b.size.x * 0.11f, b.position.y + 3.f));
    shine.setFillColor(sf::Color(220, 200, 255, 22));
    target.draw(shine);
    // Small corner accents
    sf::RectangleShape c({4.f, 4.f});
    c.setFillColor(sf::Color(255, 220, 80, 130));
    c.setPosition({b.position.x,                  b.position.y});                  target.draw(c);
    c.setPosition({b.position.x + b.size.x - 4.f, b.position.y});                  target.draw(c);
    c.setPosition({b.position.x,                  b.position.y + b.size.y - 4.f}); target.draw(c);
    c.setPosition({b.position.x + b.size.x - 4.f, b.position.y + b.size.y - 4.f}); target.draw(c);
}

// Vignette: soft dark edges that make the background feel cinematic.
static void drawVignette(sf::RenderTarget& target) {
    constexpr float W = 1280.f, H = 720.f, E = 72.f;
    constexpr uint8_t A = 105;
    sf::RectangleShape v;
    v.setFillColor(sf::Color(0, 0, 0, A));
    v.setSize(sf::Vector2f(W, E));   v.setPosition(sf::Vector2f(0.f, 0.f));     target.draw(v);
    v.setPosition(sf::Vector2f(0.f, H - E));                                     target.draw(v);
    v.setSize(sf::Vector2f(E, H));   v.setPosition(sf::Vector2f(0.f, 0.f));     target.draw(v);
    v.setPosition(sf::Vector2f(W - E, 0.f));                                     target.draw(v);
}

// ─── Layout constants (1280 × 720) ───────────────────────────────────────────
// All positions tuned to sit inside the decorative frame artwork.
namespace Layout {
    constexpr float SW = 1280.f, SH = 720.f;

    // HUD strip — three panels separated by gem/star decorations
    constexpr float HUD_Y   = 5.f,   HUD_H   = 150.f;
    constexpr float PHUD_X  = 8.f,   PHUD_W  = 388.f;   // player (left)
    constexpr float SCORE_X = 484.f, SCORE_W = 314.f;   // score  (centre)
    constexpr float EHUD_X  = 878.f, EHUD_W  = 394.f;   // enemy  (right)

    // Battle log — floats just above the question box
    constexpr float BLOG_Y = 242.f;

    // Question panel — large centre decorative box
    constexpr float QBOX_X = 78.f,  QBOX_W = 1124.f;
    constexpr float QBOX_Y = 263.f, QBOX_H  = 178.f;

    // Answer grid (2×2)
    constexpr float BTN_H   = 90.f;
    constexpr float BTN_GAP = 10.f;
    constexpr float BTN_W   = 530.f;
    constexpr float COL0_X  = 90.f;
    constexpr float COL1_X  = 660.f;
    constexpr float GRID_Y  = 447.f;   // 6px gap below question box bottom (441)
    constexpr float ROW2_Y  = 547.f;   // GRID_Y + BTN_H + BTN_GAP

    // Feedback strip (visible only when answered / hinted)
    constexpr float FB_Y = ROW2_Y + BTN_H + 4.f;   // 641
    constexpr float FB_H = 62.f;

    // Mastery bar (lives inside the bottom strip panel)
    constexpr float MBAR_X = 28.f;
    constexpr float MBAR_Y = 690.f;
    constexpr float MBAR_W = 360.f;
    constexpr float MBAR_H = 14.f;

    // Bottom control strip (original position preserved)
    constexpr float BOT_STRIP_Y = 668.f;
    constexpr float BOT_STRIP_H = 52.f;
    constexpr float BOT_Y       = 680.f;
}

// ─────────────────────────────────────────────────────────────────────────────

TrainingState::TrainingState(Game* gameInstance)
    : game(gameInstance),
      currentQuestionIndex(0), score(0), streak(0), answered(false),
      selectedOption(-1), hintUsed(false), inBattle(true),
      displayPlayerHP(100.f), targetPlayerHP(100.f), maxPlayerHP(100.f),
      displayEnemyHP(150.f),  targetEnemyHP(150.f),  maxEnemyHP(150.f),
      animTime(0.f) {

    using namespace Layout;

    correctColor  = sf::Color(60,  220, 100);
    wrongColor    = sf::Color(220, 60,  60);
    normalColor   = sf::Color(220, 220, 220);
    selectedColor = sf::Color(255, 200, 0);
    hintColor     = sf::Color(100, 170, 255);
    panelColor    = sf::Color(10,  8,   28, 215);

    if (backgroundTexture.loadFromFile("assets/images/battlefield.png")) {
        backgroundSprite.emplace(backgroundTexture);
        backgroundSprite->setScale(sf::Vector2f(
            SW / backgroundTexture.getSize().x,
            (SH) / backgroundTexture.getSize().y));
    }

    sf::Font& font = game->getMainFont();
    const sf::Color bgDark(10, 8, 28, 215);

    // ── Player HUD panel (top-left) ───────────────────────────────────────────
    playerHUDPanel = new sf::RectangleShape(sf::Vector2f(PHUD_W, HUD_H));
    playerHUDPanel->setPosition(sf::Vector2f(PHUD_X, HUD_Y));
    playerHUDPanel->setOutlineColor(sf::Color(0, 200, 200, 180));
    playerHUDPanel->setOutlineThickness(2.f);

    playerHealthText = new sf::Text(font, "LUMINARY: 100/100", 22);
    playerHealthText->setPosition(sf::Vector2f(PHUD_X + 14.f, HUD_Y + 14.f));
    playerHealthText->setFillColor(sf::Color(0, 255, 200));

    playerHealthBarBg = new sf::RectangleShape(sf::Vector2f(PHUD_W - 24.f, 16.f));
    playerHealthBarBg->setPosition(sf::Vector2f(PHUD_X + 12.f, HUD_Y + 52.f));
    playerHealthBarBg->setFillColor(sf::Color(20, 14, 38, 200));

    playerHealthBar = new sf::RectangleShape(sf::Vector2f(PHUD_W - 24.f, 16.f));
    playerHealthBar->setPosition(sf::Vector2f(PHUD_X + 12.f, HUD_Y + 52.f));
    playerHealthBar->setFillColor(sf::Color(40, 210, 180));

    playerStatText = new sf::Text(font, "ATK: 20  DEF: 10", 17);
    playerStatText->setPosition(sf::Vector2f(PHUD_X + 14.f, HUD_Y + 82.f));
    playerStatText->setFillColor(sf::Color(180, 180, 180));

    // ── Enemy HUD panel (top-right) ───────────────────────────────────────────
    enemyHUDPanel = new sf::RectangleShape(sf::Vector2f(EHUD_W, HUD_H));
    enemyHUDPanel->setPosition(sf::Vector2f(EHUD_X, HUD_Y));
    enemyHUDPanel->setOutlineColor(sf::Color(220, 60, 60, 180));
    enemyHUDPanel->setOutlineThickness(2.f);

    enemyNameText = new sf::Text(font, "FIRE DRAGON", 24);
    enemyNameText->setPosition(sf::Vector2f(EHUD_X + 12.f, HUD_Y + 12.f));
    enemyNameText->setFillColor(sf::Color(255, 80, 60));

    enemyHealthText = new sf::Text(font, "150 / 150", 20);
    enemyHealthText->setPosition(sf::Vector2f(EHUD_X + 12.f, HUD_Y + 46.f));
    enemyHealthText->setFillColor(sf::Color::White);

    enemyElementText = new sf::Text(font, "FIRE TYPE", 18);
    enemyElementText->setPosition(sf::Vector2f(EHUD_X + 12.f, HUD_Y + 74.f));
    enemyElementText->setFillColor(sf::Color(255, 140, 0));

    enemyHealthBarBg = new sf::RectangleShape(sf::Vector2f(EHUD_W - 24.f, 16.f));
    enemyHealthBarBg->setPosition(sf::Vector2f(EHUD_X + 12.f, HUD_Y + 106.f));
    enemyHealthBarBg->setFillColor(sf::Color(20, 14, 38, 200));

    enemyHealthBar = new sf::RectangleShape(sf::Vector2f(EHUD_W - 24.f, 16.f));
    enemyHealthBar->setPosition(sf::Vector2f(EHUD_X + 12.f, HUD_Y + 106.f));
    enemyHealthBar->setFillColor(sf::Color(220, 60, 60));

    // ── Centre score / streak panel ───────────────────────────────────────────
    scorePanel = new sf::RectangleShape(sf::Vector2f(SCORE_W, HUD_H));
    scorePanel->setPosition(sf::Vector2f(SCORE_X, HUD_Y));
    scorePanel->setFillColor(sf::Color(20, 20, 40, 200));
    scorePanel->setOutlineColor(sf::Color(255, 215, 0, 180));
    scorePanel->setOutlineThickness(2.f);

    // Three lines evenly distributed across HUD_H with equal top/bottom margins
    scoreText = new sf::Text(font, "SCORE: 0", 21);
    scoreText->setPosition(sf::Vector2f(SCORE_X + 16.f, HUD_Y + 18.f));
    scoreText->setFillColor(sf::Color(255, 220, 80));

    streakText = new sf::Text(font, "STREAK: 0", 21);
    streakText->setPosition(sf::Vector2f(SCORE_X + 16.f, HUD_Y + 58.f));
    streakText->setFillColor(sf::Color(255, 220, 80));

    // streakPanel: kept only for cleanup compatibility, never rendered
    streakPanel = new sf::RectangleShape(sf::Vector2f(1, 1));
    streakPanel->setPosition(sf::Vector2f(-9999, -9999));

    progressText = new sf::Text(font, "Q 1/20", 21);
    progressText->setPosition(sf::Vector2f(SCORE_X + 16.f, HUD_Y + 98.f));
    progressText->setFillColor(sf::Color(255, 220, 80));

    // ── Battle log background + text ─────────────────────────────────────────
    battleLogBg = new sf::RectangleShape(sf::Vector2f(600.f, 35.f));
    battleLogBg->setPosition(sf::Vector2f(10.f, 230.f));
    battleLogBg->setFillColor(sf::Color(0, 0, 0, 140));

    battleLogText = new sf::Text(font, "BATTLE STARTED! Answer to attack!", 21);
    battleLogText->setPosition(sf::Vector2f(20.f, BLOG_Y));
    battleLogText->setFillColor(sf::Color(100, 255, 100));

    // ── Question panel ────────────────────────────────────────────────────────
    questionBox = new sf::RectangleShape(sf::Vector2f(QBOX_W, QBOX_H));
    questionBox->setPosition(sf::Vector2f(QBOX_X, QBOX_Y));
    questionBox->setFillColor(sf::Color(10, 10, 30, 210));
    questionBox->setOutlineColor(sf::Color(120, 80, 200, 200));
    questionBox->setOutlineThickness(2.f);

    questionText = new sf::Text(font, "", 23);
    questionText->setPosition(sf::Vector2f(QBOX_X + 20.f, QBOX_Y + 16.f));
    questionText->setFillColor(sf::Color::White);

    // ── Answer buttons (2×2 grid) — right edge 1190, bottom 590, all on-screen
    optionBoxes.clear();
    optionTexts.clear();

    const float btnXY[4][2] = {
        {COL0_X, GRID_Y},   // A: ( 90, 400)
        {COL1_X, GRID_Y},   // B: (660, 400) — right edge 1190
        {COL0_X, ROW2_Y},   // C: ( 90, 500)
        {COL1_X, ROW2_Y}    // D: (660, 500) — bottom 590
    };

    for (int i = 0; i < 4; i++) {
        auto* box = new sf::RectangleShape(sf::Vector2f(BTN_W, BTN_H));
        box->setPosition(sf::Vector2f(btnXY[i][0], btnXY[i][1]));
        box->setFillColor(sf::Color(30, 15, 55, 180));
        box->setOutlineColor(sf::Color(255, 255, 255, 180));
        box->setOutlineThickness(2);
        optionBoxes.push_back(box);

        auto* text = new sf::Text(font, "", 18);
        text->setPosition(sf::Vector2f(btnXY[i][0] + 16.f, btnXY[i][1] + 33.f));
        text->setFillColor(normalColor);
        optionTexts.push_back(text);
    }

    // ── Feedback strip (below answers, above buttons) ─────────────────────────
    feedbackBox = new sf::RectangleShape(sf::Vector2f(QBOX_W, FB_H));
    feedbackBox->setPosition(sf::Vector2f(QBOX_X, FB_Y));
    feedbackBox->setFillColor(sf::Color(14, 10, 38, 148));

    feedbackText = new sf::Text(font, "", 16);
    feedbackText->setPosition(sf::Vector2f(QBOX_X + 14.f, FB_Y + 10.f));
    feedbackText->setFillColor(sf::Color::White);

    // ── Mastery bar ───────────────────────────────────────────────────────────
    masteryBar = new sf::RectangleShape(sf::Vector2f(MBAR_W, MBAR_H));
    masteryBar->setPosition(sf::Vector2f(MBAR_X, MBAR_Y));
    masteryBar->setFillColor(sf::Color(60, 30, 120, 200));
    masteryBar->setOutlineColor(sf::Color(140, 90, 220, 150));
    masteryBar->setOutlineThickness(1);

    masteryBarFill = new sf::RectangleShape(sf::Vector2f(0, MBAR_H));
    masteryBarFill->setPosition(sf::Vector2f(MBAR_X, MBAR_Y));
    masteryBarFill->setFillColor(sf::Color(140, 90, 220));

    masteryText = new sf::Text(font, "Mastery: 0%", 17);
    masteryText->setPosition(sf::Vector2f(MBAR_X, MBAR_Y - 18.f));
    masteryText->setFillColor(sf::Color(180, 180, 255));

    // ── Bottom nav ────────────────────────────────────────────────────────────
    backButtonText = new sf::Text(font, "MENU (ESC)", 18);
    backButtonText->setPosition(sf::Vector2f(22.f, BOT_Y));
    backButtonText->setFillColor(sf::Color(255, 215, 100));

    hintButtonText = new sf::Text(font, "HINT (-2) - H", 18);
    hintButtonText->setPosition(sf::Vector2f(SW / 2.f - 70.f, BOT_Y));
    hintButtonText->setFillColor(sf::Color(255, 215, 100));

    nextButtonText = new sf::Text(font, "NEXT (ENTER)", 18);
    nextButtonText->setPosition(sf::Vector2f(SW - 170.f, BOT_Y));
    nextButtonText->setFillColor(sf::Color(255, 215, 100));

    // ── Bottom divider line ───────────────────────────────────────────────────
    bottomDivider = new sf::RectangleShape(sf::Vector2f(SW, 2.f));
    bottomDivider->setPosition(sf::Vector2f(0.f, BOT_STRIP_Y));
    bottomDivider->setFillColor(sf::Color(80, 60, 120, 200));

    // Questions loaded in onEnter() after grade/subject are set
    displayQuestion();
    std::cout << "TrainingState created" << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────

void TrainingState::loadQuestions() {
    questions.clear();

    std::string grade   = game->playerData.gradeLevel;
    std::string subject = !game->selectedSubject.empty()
                          ? game->selectedSubject
                          : game->playerData.selectedSubject;

    if (grade.empty() || subject.empty()) {
        std::cerr << "[TrainingState] Grade or subject not set.\n";
        return;
    }

    std::string folderPrefix = gradeToFolderPrefix(grade);
    if (folderPrefix.empty()) {
        std::cerr << "[TrainingState] Unrecognised grade: '" << grade << "'\n";
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

    size_t qkey = content.find("\"questions\"");
    if (qkey == std::string::npos) { std::cerr << "[TrainingState] No 'questions' key.\n"; return; }
    size_t arrStart = content.find('[', qkey);
    if (arrStart == std::string::npos) return;

    size_t pos = arrStart + 1;
    while (pos < content.size()) {
        size_t objStart = content.find('{', pos);
        if (objStart == std::string::npos) break;

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
        q.hint         = jsStr(obj, "hint");
        q.subject      = subject;
        q.id           = static_cast<int>(questions.size() + 1);

        if (!q.questionText.empty() && q.options.size() == 4)
            questions.push_back(std::move(q));

        pos = objEnd;
    }

    std::mt19937 rng(std::random_device{}());
    std::shuffle(questions.begin(), questions.end(), rng);

    std::cout << "[TrainingState] Loaded " << questions.size()
              << " questions (" << grade << " / " << subject << ")\n";
}

void TrainingState::displayQuestion() {
    using namespace Layout;

    if (questions.empty()) {
        questionText->setString("No questions loaded for this subject/grade.\nPress ESC to return.");
        for (int i = 0; i < 4; i++) optionTexts[i]->setString("");
        progressText->setString("0/0");
        return;
    }
    if (currentQuestionIndex >= (int)questions.size()) return;

    sf::Font& font = game->getMainFont();
    Question& q = questions[currentQuestionIndex];

    // Wrap question text to fit within the panel (padding: 40px total)
    questionText->setString(wrapText(q.questionText, font, 22, QBOX_W - 40.f));

    // Wrap each option text to fit within a button (14px left + 14px right padding)
    for (int i = 0; i < 4; i++) {
        std::string prefix = std::string(1, char('A' + i)) + ". ";
        std::string wrapped = wrapText(prefix + q.options[i], font, 18, BTN_W - 28.f);
        optionTexts[i]->setString(wrapped);
        optionTexts[i]->setFillColor(normalColor);
        optionBoxes[i]->setFillColor(sf::Color(30, 15, 55, 180));
        optionBoxes[i]->setOutlineColor(sf::Color(160, 120, 35, 160));
        optionBoxes[i]->setOutlineThickness(2);
    }

    progressText->setString("Q " + std::to_string(currentQuestionIndex + 1)
                           + "/" + std::to_string(questions.size()));

    float masteryPct = (score > 0 && currentQuestionIndex > 0)
                       ? std::min(100.f, (score / ((currentQuestionIndex + 1) * 15.f)) * 100.f)
                       : 0.f;
    masteryBarFill->setSize(sf::Vector2f(MBAR_W * (masteryPct / 100.f), MBAR_H));
    masteryText->setString("Mastery: " + std::to_string((int)masteryPct) + "%");

    feedbackText->setString("");
    feedbackBox->setFillColor(sf::Color(18, 12, 40, 210));
    answered      = false;
    selectedOption = -1;
    hintUsed      = false;
    updateUI();
}

void TrainingState::checkAnswer(int optionIndex) {
    if (answered) return;

    answered       = true;
    selectedOption = optionIndex;
    Question& q    = questions[currentQuestionIndex];

    game->playerData.totalQuestionsAnswered++;

    if (optionIndex == q.correctIndex) {
        game->playerData.correctAnswers++;
        int points = 10 + (streak * 2);
        score  += points;
        streak++;

        screenShake.start(0.2f, 6.0f);
        particles.spawnSparkles(optionBoxes[optionIndex]->getPosition());

        // Correct button — green
        optionBoxes[optionIndex]->setFillColor(sf::Color(10, 55, 22, 220));
        optionBoxes[optionIndex]->setOutlineColor(sf::Color(60, 220, 100));
        optionBoxes[optionIndex]->setOutlineThickness(3);
        optionTexts[optionIndex]->setFillColor(sf::Color(100, 255, 140));

        battleLogText->setString("CORRECT! +" + std::to_string(points) + " points!");
        battleLogText->setFillColor(sf::Color(100, 255, 160));

        std::string fbStr = "CORRECT!  +" + std::to_string(points) + " pts     " + q.explanation;
        feedbackText->setString(wrapText(fbStr, game->getMainFont(), 16, Layout::QBOX_W - 28.f));
        feedbackBox->setFillColor(sf::Color(6, 38, 16, 155));

        std::cout << "Correct! Streak: " << streak << " Score: " << score << "\n";
    } else {
        streak = 0;

        screenShake.start(0.2f, 4.0f);
        particles.spawnSmoke(optionBoxes[optionIndex]->getPosition());

        // Wrong button — crimson
        optionBoxes[optionIndex]->setFillColor(sf::Color(55, 8, 8, 220));
        optionBoxes[optionIndex]->setOutlineColor(sf::Color(220, 60, 60));
        optionBoxes[optionIndex]->setOutlineThickness(3);
        optionTexts[optionIndex]->setFillColor(sf::Color(255, 100, 100));

        // Reveal correct answer — green outline
        optionBoxes[q.correctIndex]->setOutlineColor(sf::Color(60, 220, 100));
        optionBoxes[q.correctIndex]->setOutlineThickness(3);
        optionTexts[q.correctIndex]->setFillColor(sf::Color(100, 255, 140));

        battleLogText->setString("WRONG! Enemy attacks!");
        battleLogText->setFillColor(sf::Color(255, 100, 100));

        std::string fbStr = "INCORRECT     Correct: " + q.options[q.correctIndex]
                            + "     " + q.explanation;
        feedbackText->setString(wrapText(fbStr, game->getMainFont(), 16, Layout::QBOX_W - 28.f));
        feedbackBox->setFillColor(sf::Color(42, 6, 6, 155));

        std::cout << "Wrong! Streak reset. Score: " << score << "\n";
    }

    updateUI();
    saveProgress();
}

void TrainingState::nextQuestion() {
    if (!answered) return;

    currentQuestionIndex++;

    if (currentQuestionIndex >= (int)questions.size()) {
        feedbackText->setString(wrapText(
            "CHAPTER COMPLETE!    Final Score: " + std::to_string(score),
            game->getMainFont(), 16, Layout::QBOX_W - 28.f));
        nextButtonText->setString("PRESS ENTER FOR MENU");
        particles.spawnSparkles(sf::Vector2f(640, 360));
        battleLogText->setString("VICTORY!");
        battleLogText->setFillColor(sf::Color(255, 220, 80));
    } else {
        displayQuestion();
        battleLogText->setString("Next question! Answer to attack!");
        battleLogText->setFillColor(sf::Color(220, 220, 100));
    }
}

void TrainingState::showHint() {
    if (answered || hintUsed) return;

    hintUsed = true;
    score -= 2;
    if (score < 0) score = 0;

    Question& q = questions[currentQuestionIndex];
    std::string hint = q.hint.empty()
                       ? "Eliminate obviously wrong choices first."
                       : q.hint;
    std::string fbStr = "HINT: " + hint + "  (-2 points)";
    feedbackText->setString(wrapText(fbStr, game->getMainFont(), 16, Layout::QBOX_W - 28.f));
    feedbackBox->setFillColor(sf::Color(28, 18, 65, 155));
    updateUI();
    saveProgress();
}

void TrainingState::startBattle() {
    inBattle = true;
    battleLogText->setString("BATTLE STARTED! Answer to attack!");
    battleLogText->setFillColor(sf::Color(220, 220, 100));
}

void TrainingState::updateUI() {
    scoreText->setString("SCORE: " + std::to_string(score));

    streakText->setString("STREAK: " + std::to_string(streak));
    if      (streak >= 8) streakText->setFillColor(sf::Color(255, 80,  80));
    else if (streak >= 5) streakText->setFillColor(sf::Color(255, 160, 40));
    else if (streak >= 3) streakText->setFillColor(sf::Color(255, 220, 80));
    else                  streakText->setFillColor(sf::Color(180, 220, 255));
}

void TrainingState::updateBattleUI() {}

void TrainingState::saveProgress() {
    if (!game->hasLoadedProfile && game->playerData.username.empty()) return;
    game->playerData.totalScore = score;
    game->playerData.bestStreak = std::max(game->playerData.bestStreak, streak);
    SaveManager::savePlayer(game->playerData);
}

// ─────────────────────────────────────────────────────────────────────────────

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
    animTime += deltaTime;
    screenShake.update(deltaTime);
    particles.update(deltaTime);

    // Lerp HP bars toward target values
    float t = std::min(1.f, deltaTime * 5.f);
    displayPlayerHP = lerpF(displayPlayerHP, targetPlayerHP, t);
    displayEnemyHP  = lerpF(displayEnemyHP,  targetEnemyHP,  t);

    if (maxPlayerHP > 0.f) {
        float ratio = displayPlayerHP / maxPlayerHP;
        float w = playerHealthBarBg->getSize().x * ratio;
        playerHealthBar->setSize({w, playerHealthBar->getSize().y});
    }
    if (maxEnemyHP > 0.f) {
        float ratio = displayEnemyHP / maxEnemyHP;
        float w = enemyHealthBarBg->getSize().x * ratio;
        enemyHealthBar->setSize({w, enemyHealthBar->getSize().y});
    }
}

void TrainingState::render(sf::RenderWindow& window) {
    using namespace Layout;

    // Screen shake
    sf::View view = window.getView();
    sf::Vector2f shakeOffset = screenShake.getOffset();
    view.setCenter(sf::Vector2f(640.f + shakeOffset.x, 360.f + shakeOffset.y));
    window.setView(view);

    // Reusable animation values
    const float slowPulse  = (std::sin(animTime * 1.2f) + 1.f) * 0.5f;   // 0..1
    const float midPulse   = (std::sin(animTime * 2.0f) + 1.f) * 0.5f;
    const float hudGlow    = slowPulse * 0.35f;
    const float qGlow      = (std::sin(animTime * 0.9f) + 1.f) * 0.5f * 0.25f;

    // Glass panel backgrounds
    const sf::Color playerHudBg(8,  12, 32, 148);
    const sf::Color enemyHudBg (26,  7,  8, 148);
    const sf::Color panelBg    (14, 10, 38, 145);

    // ── 1. Background ─────────────────────────────────────────────────────────
    if (backgroundSprite.has_value()) window.draw(*backgroundSprite);

    // ── 2. Cinematic vignette ─────────────────────────────────────────────────
    drawVignette(window);

    // ── 3. HUD panels ─────────────────────────────────────────────────────────
    drawGlassPanel(window, playerHUDPanel->getGlobalBounds(), hudGlow, playerHudBg);
    drawGlassPanel(window, enemyHUDPanel->getGlobalBounds(),  hudGlow, enemyHudBg);
    drawGlassPanel(window, scorePanel->getGlobalBounds(),     hudGlow, panelBg);

    // ── 4. HP bars with gradient shine overlay ────────────────────────────────
    window.draw(*playerHealthBarBg);
    window.draw(*playerHealthBar);
    {
        sf::RectangleShape shine(sf::Vector2f(playerHealthBar->getSize().x,
                                              playerHealthBar->getSize().y * 0.45f));
        shine.setPosition(playerHealthBar->getPosition());
        shine.setFillColor(sf::Color(190, 255, 240, 45));
        window.draw(shine);
    }
    window.draw(*enemyHealthBarBg);
    window.draw(*enemyHealthBar);
    {
        sf::RectangleShape shine(sf::Vector2f(enemyHealthBar->getSize().x,
                                              enemyHealthBar->getSize().y * 0.45f));
        shine.setPosition(enemyHealthBar->getPosition());
        shine.setFillColor(sf::Color(255, 190, 160, 45));
        window.draw(shine);
    }

    // ── 5. HUD text ───────────────────────────────────────────────────────────
    drawShadowText(window, *playerHealthText, playerHealthText->getPosition());
    drawShadowText(window, *playerStatText,   playerStatText->getPosition());
    drawShadowText(window, *enemyNameText,    enemyNameText->getPosition());
    drawShadowText(window, *enemyHealthText,  enemyHealthText->getPosition());
    drawShadowText(window, *enemyElementText, enemyElementText->getPosition());
    drawShadowText(window, *scoreText,        scoreText->getPosition());
    drawShadowText(window, *streakText,       streakText->getPosition());
    drawShadowText(window, *progressText,     progressText->getPosition());

    // ── 6. Battle log ─────────────────────────────────────────────────────────
    window.draw(*battleLogBg);
    drawShadowText(window, *battleLogText, battleLogText->getPosition());

    // ── 7. Question panel ─────────────────────────────────────────────────────
    drawGlassPanel(window, questionBox->getGlobalBounds(), qGlow, sf::Color(12, 8, 36, 152));
    drawShadowText(window, *questionText, questionText->getPosition());

    // ── 8. Answer buttons — animated idle glow, states on answer ─────────────
    for (int i = 0; i < 4; i++) {
        // Staggered idle border animation when question is still open
        if (!answered) {
            if (selectedOption == i) {
                optionBoxes[i]->setOutlineColor(sf::Color(255, 200, 0, 255));
                optionBoxes[i]->setOutlineThickness(3.f);
            } else {
                float phase = (std::sin(animTime * 2.4f + i * 0.85f) + 1.f) * 0.5f;
                auto c = [](int base, int add, float t) -> uint8_t {
                    int v = base + (int)(t * add);
                    return static_cast<uint8_t>(v > 255 ? 255 : v);
                };
                optionBoxes[i]->setOutlineColor(sf::Color(
                    c(120, 80, phase), c(88, 62, phase), c(28, 18, phase),
                    c(130, 80, phase)
                ));
                optionBoxes[i]->setOutlineThickness(2.f);
            }
        }
        window.draw(*optionBoxes[i]);
        drawButtonShine(window, optionBoxes[i]->getGlobalBounds());
        drawShadowText(window, *optionTexts[i], optionTexts[i]->getPosition());
    }

    // ── 9. Feedback strip ─────────────────────────────────────────────────────
    if (answered || hintUsed) {
        sf::Color fbBg = feedbackBox->getFillColor();
        drawGlassPanel(window, feedbackBox->getGlobalBounds(), 0.45f, fbBg);
        drawShadowText(window, *feedbackText, feedbackText->getPosition());
    }

    // ── 10. Mastery bar ───────────────────────────────────────────────────────
    window.draw(*masteryBar);
    window.draw(*masteryBarFill);
    // Animated shimmer on mastery fill — clipped to bar bounds
    if (masteryBarFill->getSize().x > 8.f) {
        const float shimW  = 28.f;
        const float barL   = masteryBarFill->getPosition().x;
        const float barR   = barL + masteryBarFill->getSize().x;
        const float travel = masteryBarFill->getSize().x + shimW;
        float shimX = barL + std::fmod(animTime * 50.f, travel) - shimW;
        float visL = std::max(shimX, barL);
        float visR = std::min(shimX + shimW, barR);
        if (visR > visL) {
            sf::RectangleShape shimmer(sf::Vector2f(visR - visL,
                                                    masteryBarFill->getSize().y));
            shimmer.setPosition(sf::Vector2f(visL, masteryBarFill->getPosition().y));
            shimmer.setFillColor(sf::Color(255, 255, 255, 28));
            window.draw(shimmer);
        }
    }
    drawShadowText(window, *masteryText, masteryText->getPosition());

    // ── 11. Bottom control strip ──────────────────────────────────────────────
    float stripGlow = midPulse * 0.2f;
    drawGlassPanel(window,
                   sf::FloatRect(sf::Vector2f(0.f, BOT_STRIP_Y),
                                 sf::Vector2f(SW,  BOT_STRIP_H)),
                   stripGlow,
                   sf::Color(10, 8, 28, 130));
    window.draw(*bottomDivider);
    drawShadowText(window, *backButtonText, backButtonText->getPosition());
    drawShadowText(window, *hintButtonText, hintButtonText->getPosition());
    drawShadowText(window, *nextButtonText, nextButtonText->getPosition());

    // ── 12. Particles ─────────────────────────────────────────────────────────
    particles.render(window);

    window.setView(window.getDefaultView());
}

// ─────────────────────────────────────────────────────────────────────────────

void TrainingState::cleanup() {
    delete playerHUDPanel;
    delete playerHealthText;
    delete playerHealthBarBg;
    delete playerHealthBar;
    delete playerStatText;
    delete enemyHUDPanel;
    delete enemyNameText;
    delete enemyHealthText;
    delete enemyHealthBarBg;
    delete enemyHealthBar;
    delete enemyElementText;
    delete battleLogBg;
    delete battleLogText;
    delete bottomDivider;
    delete questionBox;
    delete questionText;
    for (auto* b : optionBoxes)  delete b;
    for (auto* t : optionTexts)  delete t;
    optionBoxes.clear();
    optionTexts.clear();
    delete scorePanel;
    delete scoreText;
    delete streakPanel;
    delete streakText;
    delete progressText;
    delete feedbackBox;
    delete feedbackText;
    delete masteryBar;
    delete masteryBarFill;
    delete masteryText;
    delete nextButtonText;
    delete backButtonText;
    delete hintButtonText;
}

TrainingState::~TrainingState() {
    cleanup();
}

void TrainingState::onEnter() {
    std::cout << "Entered Training State\n";
    currentQuestionIndex = 0;
    score    = 0;
    streak   = 0;
    answered = false;
    inBattle = true;
    animTime = 0.f;

    // Reset HP display
    displayPlayerHP = targetPlayerHP = maxPlayerHP = 100.f;
    displayEnemyHP  = targetEnemyHP  = maxEnemyHP  = 150.f;
    playerHealthBar->setSize({playerHealthBarBg->getSize().x, playerHealthBar->getSize().y});
    enemyHealthBar ->setSize({enemyHealthBarBg->getSize().x,  enemyHealthBar->getSize().y});

    loadQuestions();
    startBattle();
    displayQuestion();
}

void TrainingState::onExit() {
    std::cout << "Exited Training State\n";
    saveProgress();
}
