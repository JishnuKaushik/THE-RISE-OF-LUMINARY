# THE RISING OF THE LUMINARY

## Project Overview

A cinematic educational RPG / roguelike deck-builder built in **C++17** with **SFML 3.0.2**.

Learning is integrated directly into gameplay. Players answer educational questions during combat — correct answers trigger attacks, combo streaks, critical hits, particles, XP gain, and progression. The goal is to make learning feel like gameplay, not a quiz.

---

## Tech Stack

| Tool | Version |
|---|---|
| C++ | 17 |
| SFML | 3.0.2 |
| Build | CMake 3.10+ |
| JSON | nlohmann/json v3.11.3 |
| Compiler | MinGW64 GCC 16.1.0 |
| Platform | Windows |

Architecture: state-machine, data-driven, JSON content pipelines, OOP.

**Build Command:**
```bash
cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --target LuminaryGame
./LuminaryGame.exe
```

> Always run the executable from the project root — all asset paths are relative.

---

## Game Flow

```
SPLASH → MAIN MENU → PROFILE SELECTION → REGISTRATION
       → CHARACTER SELECTION → TRAINING / BATTLE → STORY MODE
```

---

## Project Structure

```
src/                          — all C++ source files
assets/
  fonts/                      — arial.ttf
  images/                     — character/background art, card art
  sounds/                     — theme song
  video/frames/               — intro animation frames
data/
  questions/
    Class_Preschool/          — Preschool_{Subject}.json
    Class_Elementary/         — Elementary_{Subject}.json
    Class_MiddleSchool/       — MiddleSchool_{Subject}.json
    Class_HighSchool/         — HighSchool_{Subject}.json
    Class_CollegePrep/        — CollegePrep_{Subject}.json
  stories/
    {Subject}_Stories.json    — 30-chapter story content per subject
saves/                        — JSON player save files
```

> Note: there are `src_BACKUP/` and `data_BACKUP/` directories — do not touch them. Also ignore `TrainingState_backup_filter.cpp`, `TrainingState_filtered.cpp`, and `TrainingState_new_load.cpp`; only `TrainingState.cpp` is active.

---

## Key Source Files

| File | Role |
|---|---|
| `Game.cpp/hpp` | Main loop, state switching, shared player data |
| `TrainingState.cpp/hpp` | Core battle + question system (active) |
| `TrainingStoryState.cpp/hpp` | Story mode battle integration |
| `BattleSystem.cpp/hpp` | Turn-based combat logic |
| `Character.cpp/hpp` | Player character stats and progression |
| `Enemy.cpp/hpp` | Enemy generation |
| `SaveManager.cpp/hpp` | JSON save/load for profiles |
| `ParticleSystem.cpp/hpp` | Visual particle effects |
| `ScreenShake.cpp/hpp` | Camera shake feedback |
| `ElementSystem.cpp/hpp` | Elemental combat bonuses |
| `StoryManager.hpp` | Story JSON loading |
| `ParallaxBackground.cpp/hpp` | Scrolling battle backgrounds |

---

## State Machine (Game.hpp)

Current enum:
```cpp
enum class GameState {
    SPLASH,
    MENU,
    REGISTRATION,
    TRAINING,
    GAMEPLAY,
    PROFILE_SELECTION,
    CHARACTER_SELECTION
    // STORY_MODE must be added — see BUG-002
};
```

State flow:
```
SPLASH → MENU → REGISTRATION → CHARACTER_SELECTION → TRAINING
              →
         PROFILE_SELECTION → CHARACTER_SELECTION → TRAINING
                                                  → STORY_MODE (to be added)
```

**CRITICAL RULE:** Adding any new `GameState` enum value requires updating ALL THREE switch blocks in `Game.cpp`: `handleEvents()`, `update()`, `render()`. Also requires a new `switchTo*()` method in `Game.hpp` and its implementation in `Game.cpp`. Missing any one of these causes silent state corruption or crashes.

---

## Shared Game State (Game.hpp)

```cpp
std::string selectedSubject;   // set during subject selection
PlayerData  playerData;        // includes gradeLevel
bool        hasLoadedProfile;
int         selectedCharacterId;
```

These are the two fields that drive question filtering:
- `game->selectedSubject` — matches the subject prefix in question filenames
- `game->playerData.gradeLevel` — maps to a `Class_*` folder name

---

## Concrete Data Structures

### PlayerData (SaveManager.hpp)
```cpp
struct PlayerData {
    std::string username;
    int age;
    std::string gradeLevel;       // "Preschool" | "Elementary" | "Middle School"
                                  // "High School" | "College Prep"
    std::string selectedSubject;  // "Mathematics" | "Science" | "History"
                                  // "Literature" | "Arts"
    int totalScore;
    int bestStreak;
    int totalQuestionsAnswered;
    int correctAnswers;
    std::vector<int> completedChapters;  // already exists, use it
    std::string lastPlayedDate;
    int currentStreak;
    int dailyMinutes;
    int selectedCharacter;
};
```

### Character (Character.hpp)
```cpp
struct Character {
    int id;
    std::string name;
    Element element;        // FIRE | WATER | LIGHTNING | NATURE | DARK | LIGHT
    Rarity rarity;          // COMMON | RARE | EPIC | LEGENDARY
    int health, maxHealth;
    int attack, defense;
    int level;
    int currentXP;
    int unlockCost;
    bool isUnlocked;
    std::string spriteName;
    std::string specialAbility;
    std::string abilityDescription;

    // Already implemented:
    int getRequiredXPForNextLevel() const;
    void addXP(int amount);
    void levelUp();
};
```

### CharacterManager (Character.hpp) — BUILT, NOT WIRED
```cpp
class CharacterManager {
    // Key methods already implemented:
    bool unlockCharacter(int characterId);
    bool selectCharacter(int characterId);
    Character* getSelectedCharacter();
    void addLuminescence(int amount);
    bool spendLuminescence(int amount);
    void saveProgress();    // saves to saves/characters.json
    void loadProgress();
};
```

### Card / CardManager (Card.hpp) — BUILT, NOT WIRED
```cpp
struct Card {
    int id;
    std::string name;
    CardRarity rarity;      // COMMON | RARE | EPIC | LEGENDARY
    CardElement element;
    int damage;
    int healAmount;
    int manaCost;
    std::string description;
    std::string specialEffect;
    bool isOwned;
};

class CardManager {
    // Already implemented:
    void drawCard();
    void drawInitialHand();
    void addCardToDeck(int cardId);
    bool playCard(int cardIndex);
    std::vector<Card> getCurrentHand() const;
};
```

### Enemy / BattleSystem (BattleSystem.hpp) — EXISTS, PARTIALLY WIRED
```cpp
struct Enemy {
    std::string name;
    Element element;
    int health, maxHealth;
    int attack, defense;
    int level;
    int luminescenceReward;
    int xpReward;           // already has reward field, use it
    std::string spriteName;
};

class BattleSystem {
    // Already implemented:
    void generateRandomEnemy(int playerLevel);
    void playerAttack(int damage, Element attackElement);
    void enemyAttack();
    float calculateElementMultiplier(Element attacker, Element defender);
};
```

### StoryManager (StoryManager.hpp) — BUILT, NOT WIRED
```cpp
struct StoryChapter {
    int chapterNumber;
    std::string chapterName;
    std::string storyIntro;
    std::string conceptExplanation;
    std::string realLifeExample;
    struct {
        std::string question;
        std::vector<std::string> options;
        int correct;
        std::string explanation;
    } quickCheck;
};

class StoryManager {
    // Already implemented:
    void loadAllStories();
    std::vector<StoryChapter> getStoriesBySubject(const std::string& subject);
    int getTotalChapters(const std::string& subject);
};
```

### Question struct (TrainingState.hpp/cpp)
```cpp
struct Question {
    int id;
    std::string subject;
    std::string questionText;
    std::vector<std::string> options;
    int correctIndex;        // 'A'=0, 'B'=1, 'C'=2, 'D'=3
    std::string explanation;
    std::string funFact;
    std::string hint;
};
```

---

## Question / Data System

**24 JSON question files, 440+ questions** across 5 subjects × 5 grade levels.

Subjects: `Mathematics`, `Science`, `History`, `Literature`, `Arts`

Grade folders:
```
Class_Preschool
Class_Elementary
Class_MiddleSchool
Class_HighSchool
Class_CollegePrep
```

File naming pattern: `data/questions/Class_{Grade}/{Grade}_{Subject}.json`

Example for Middle School + Mathematics:
```
data/questions/Class_MiddleSchool/MiddleSchool_Math.json
```

**Question filtering is already implemented** in `TrainingState::loadQuestions()` (lines ~407–479). It reads `game->playerData.gradeLevel` and `game->selectedSubject` (falling back to `game->playerData.selectedSubject`), runs them through `gradeToFolderPrefix()` and `subjectToFilename()`, and opens exactly one file. Do not re-implement this.

### Grade → Folder Mapping (gradeToFolderPrefix)
```
"Preschool"     → Class_Preschool/Preschool_
"Elementary"    → Class_Elementary/Elementary_
"Middle School" → Class_MiddleSchool/MiddleSchool_
"High School"   → Class_HighSchool/HighSchool_
"College Prep"  → Class_CollegePrep/CollegePrep_
```

### Subject → File Suffix Mapping (subjectToFilename)
```
"Mathematics"  → Math      (only this one is different)
"Science"      → Science
"History"      → History
"Literature"   → Literature
"Arts"         → Arts
```

### JSON Question Format
```json
{
  "subject": "Mathematics",
  "grade": "Elementary School",
  "questions": [
    {
      "id": 1,
      "question": "What is 8 + 7?",
      "options": ["14", "15", "16", "17"],
      "correct": "B",
      "explanation": "8 + 7 = 15",
      "funFact": "The + symbol was first used in 1489."
    }
  ]
}
```

---

## Story System

5 story files, 30 chapters each, one per subject:
```
data/stories/Mathematics_Stories.json
data/stories/Science_Stories.json
data/stories/History_Stories.json
data/stories/Literature_Stories.json
data/stories/Arts_Stories.json
```

Story loading is functional. Story-battle integration is partially complete.

### JSON Story Format
```json
{
  "subject": "Mathematics",
  "chapters": [
    {
      "chapterNumber": 1,
      "chapterName": "The Mystery of the Missing Variable",
      "storyIntro": "...",
      "conceptExplanation": "...",
      "realLifeExample": "...",
      "quickCheck": {
        "question": "If y = 5, what is the value of 2y?",
        "options": ["2", "5", "10", "25"],
        "correct": 2,
        "explanation": "2y means 2 × y = 10"
      }
    }
  ]
}
```

---

## Open Bug Registry

> Update this section as bugs are fixed. Mark fixed bugs with **[FIXED]** and note the date + commit.

### BUG-001 [FIXED] — Subject + Grade Filtering Not Working
- **File:** `TrainingState.cpp` → `loadQuestions()`
- **Root cause:** Was iterating ALL `Class_*` folders; now loads single targeted file.
- **Status:** Fixed (already resolved in codebase before Phase 0 work)

### BUG-002 [FIXED] — Story Mode Not Accessible
- **Files:** `Game.hpp`, `Game.cpp`, `MenuState.cpp`, `TrainingStoryState.cpp/hpp`, `StoryManager.cpp` (new)
- **Root cause:** `STORY_MODE` never in `GameState` enum; `TrainingStoryState` was a stub.
- **Fix applied:** Added `STORY_MODE` enum + `switchToStoryMode()` + all 3 switch blocks in `Game.cpp`; full 7-screen `TrainingStoryState` using `StoryManager`; "STORY MODE" added to menu at index 3.
- **Status:** Fixed

### BUG-003 [FIXED] — XP Awarded But Not Persisted
- **File:** `TrainingState.cpp` → `checkAnswer()`, `nextQuestion()`
- **Root cause:** `Character::addXP()` and `CharacterManager::saveProgress()` existed but were never called in battle
- **Fix:** Added `CharacterManager charManager` to `TrainingState`; `charManager.loadProgress()` on `onEnter()`; `ch->addXP(points)` + `charManager.saveProgress()` on correct answer; level-up detection updates `battleLogText`; +50 XP victory bonus on chapter complete
- **Status:** Fixed

### BUG-004 [FIXED] — CardManager Initialized But Never Used in Battle
- **File:** `TrainingState.cpp`, `TrainingState.hpp`
- **Root cause:** `CardManager` class was complete but no instance existed in `TrainingState`
- **Fix:** Added `CardManager cardManager` to `TrainingState`; `CardManager::initialize()` is called in constructor (draws initial hand of 3); streak milestones 3/5/7 award cards 4/5/6 via `addCardToDeck()` and display gold "CARD EARNED" message in battle log
- **Status:** Fixed

---

## Development Roadmap

Implement in this exact order. Never jump ahead.

### PHASE 0 — Critical Bug Fixes (do these before anything else)

**0.1 Subject + Grade Filtering (BUG-001)**
- Replace directory scan in `loadQuestions()` with single targeted file load
- Filter key: `game->playerData.gradeLevel` + `game->selectedSubject`
- Fallback: if subject file missing for that grade, load all subjects for that grade
- Verify: Preschool Math player gets only `Class_Preschool_Math.json` questions

**0.2 Story Mode Integration (BUG-002)**
Add to `Game.hpp`:
```cpp
// In enum:
STORY_MODE

// New method declaration:
void switchToStoryMode();

// New member:
std::unique_ptr<TrainingStoryState> trainingStoryState;
```
Wire into all switch blocks in `Game.cpp`.
Add "STORY MODE" option to `MenuState`.
Implement `TrainingStoryState` using existing `StoryManager`:
- Display `storyIntro` as scrollable text panel
- Show `conceptExplanation` after intro
- Show `quickCheck` as a question (reuse question display UI)
- Save chapter completion to `playerData.completedChapters`

---

### PHASE 1 — Wire Existing Systems Into Battle [COMPLETE]

**1.1 XP + Character Progression (BUG-003) [FIXED]**
- `CharacterManager charManager` added to `TrainingState`; `loadProgress()` on `onEnter()`
- `ch->addXP(points)` + `saveProgress()` on correct answer; level-up detected and shown in battle log
- +50 XP victory bonus on chapter complete in `nextQuestion()`

**1.2 Lite Card Integration (BUG-004) [FIXED]**
- `CardManager cardManager` added to `TrainingState`
- Streak milestones 3/5/7 award cards 4/5/6 via `addCardToDeck()` + gold "CARD EARNED" message

**1.3 Mouse Support [DONE]**
- `TrainingState`: answer boxes highlight on hover, click to answer, click when answered to advance
- `TrainingStoryState`: click subject boxes, chapter rows, answer boxes, reading screens to advance
- Keyboard coexists with all mouse input

---

### PHASE 2 — Educational Features [PARTIALLY COMPLETE]

**2.1 Weak Topic Tracking [DONE]**
- `std::map<std::string, int> topicAttempts` and `topicCorrect` added to `PlayerData`
- Incremented in `TrainingState::checkAnswer()` on every attempt and correct answer
- Serialized/deserialized in `SaveManager` using flat keys `topicAttempts_{Subject}` / `topicCorrect_{Subject}`

**2.2 Adaptive Difficulty [DONE]**
- After `gradeToFolderPrefix()`, calculates subject accuracy from `topicAttempts`/`topicCorrect` (requires ≥10 attempts)
- accuracy < 50%: tries the grade folder one step down; accuracy > 80%: tries one step up
- Falls back to original grade if the adjacent file doesn't exist

**2.3 Statistics Screen [DONE]**
- New `StatisticsState` wired into all 3 switch blocks + `switchToStatistics()`
- Shows: player name, grade, subject, questions, accuracy, streaks, daily minutes, chapters done
- Per-subject accuracy with strongest/weakest subject detection
- Click or ESC/ENTER to return to menu
- Menu index 4 (STATISTICS) now routes to this state

---

### PHASE 3 — Progression + Retention [COMPLETE]

**3.1 Achievements [DONE]**
- `AchievementManager` loads 10 achievements from `data/achievements.json` (fallback hardcoded)
- Saves per-user to `saves/{username}_achievements.json`
- `game->achievements` is a public member of `Game`; `setUsername()` called on entering Training/Story
- Checks wired in `TrainingState::checkAnswer()` (first correct, streak 5/10, questions 50/100, level 5/10, all subjects) and `nextQuestion()` (first win)
- `TrainingStoryState::saveChapterCompletion()` calls `checkStoryChapter()`
- Unlock popup via `spawnFloatingText("ACHIEVEMENT: ...")` using the callback

**3.2 Daily Quests [DONE]**
- `dailyQuestionsAnswered`, `dailyChaptersCompleted`, `dailyQuestStreakDone` added to `PlayerData`
- Saved/loaded in `SaveManager`; reset on new calendar day in `switchToTraining()` and `switchToStoryMode()`
- Three targets: 10 questions today, 1 story chapter today, 2-day login streak
- Displayed in `StatisticsState` with `[X]` / `[ ]` progress indicators

**3.3 Character Milestone Upgrades [DONE]**
- Milestone bonuses in `Character::levelUp()`: Lv.5 +5 attack, Lv.10 +3 defense, Lv.15 +20 maxHealth
- Logged to stdout with milestone message

---

### PHASE 4 — Polish [COMPLETE]

**4.1 Screen Fade Transitions [DONE]**
- `fadeOverlay`, `fadeAlpha`, `fadingOut/In`, `fadeTimer`, `pendingStateSwitch` added to `Game`
- `startFade(lambda)` called from `MenuState` keyboard/mouse handlers; all 5 menu options fade
- `Game::update()` drives fade timer; `Game::render()` draws overlay after state render, before `display()`
- `render()` refactored: single `window.display()` after all states, splash handled separately

**4.2 Floating Combat Text [DONE]**
- `ParticleSystem` extended with `FloatingText` struct (stores raw data, renders `sf::Text` each frame to avoid SFML 3 default constructor issue)
- `spawnFloatingText(pos, str, color)` requires `setFont()` to be called first (done in `TrainingState::onEnter()`)
- Spawns: green "+N" on correct, red "WRONG!" on wrong, gold "LEVEL UP!" on level up
- Fades out alpha in last 40% of lifetime (1.2s total)

**4.3 Survival Mode [DONE]**
- `bool survivalMode` + `int survivalLives` added to `TrainingState`
- Default: off. Wrong answer decrements lives; at 0 shows GAME OVER and sends to menu
- Lives displayed as hearts in the streak/lives panel when active
- Toggle: set `trainingState->survivalMode = true` before `switchToTraining()`

**4.4 Endless Mode [DONE]**
- `bool endlessMode` + `int endlessLoopCount` added to `TrainingState`
- When all questions exhausted with `endlessMode = true`: reshuffles + loops instead of ending
- Toggle: set `trainingState->endlessMode = true` before `switchToTraining()`

**4.5 Settings Menu [DONE]**
- `SettingsState` with 3 panel options: Music Volume (LEFT/RIGHT ±5), SFX Volume (LEFT/RIGHT ±5), Fullscreen toggle
- Saves to `saves/settings.json` on exit; loads on enter
- Menu now has 7 options: NEW GAME / LOAD PROFILE / TRAINING / STORY / STATISTICS / SETTINGS / EXIT
- Selected option highlighted with gold border; visual bar `[===   ] 60` for volumes
- Mouse click on panels toggles/selects; all navigation via keyboard also works

---

## Coding Standards

**Language:** C++17 — use STL containers, RAII, `std::unique_ptr`. No raw `new`/`delete` in new code.

**Naming:**
- Classes: `PascalCase`
- Methods: `camelCase`
- Constants: `UPPER_CASE`

**SFML 3.0.2 — use v3 APIs only.** This project is SFML 3 ONLY. SFML 2.x syntax causes compiler errors.

### SFML 3 Syntax Contract

```cpp
// POSITIONS — CORRECT (SFML 3)
sprite.setPosition(sf::Vector2f(x, y));
// WRONG (SFML 2) — will not compile
sprite.setPosition(x, y);

// TEXT CONSTRUCTION — CORRECT (SFML 3)
sf::Text text(font, "hello", 18);
// WRONG (SFML 2)
sf::Text text; text.setFont(font); text.setString("hello");

// SPRITE CONSTRUCTION — CORRECT (SFML 3) — requires texture at construction
sf::Sprite sprite(texture);
// WRONG (SFML 2) — no default constructor in SFML 3
sf::Sprite sprite; sprite.setTexture(texture);

// FONT LOADING — CORRECT (SFML 3)
font.openFromFile("assets/fonts/arial.ttf");
// WRONG (SFML 2)
font.loadFromFile("...");

// EVENT HANDLING — CORRECT (SFML 3)
if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    if (kp->code == sf::Keyboard::Key::Escape) { ... }
if (const auto* me = event.getIf<sf::Event::MouseButtonPressed>()) {
    sf::Vector2f pos(me->position.x, me->position.y);
}
if (const auto* te = event.getIf<sf::Event::TextEntered>()) {
    if (te->unicode < 128) { char c = static_cast<char>(te->unicode); }
}
// WRONG (SFML 2) — will not compile
if (event.type == sf::Event::KeyPressed) { ... }

// BOUNDS — CORRECT (SFML 3)
getLocalBounds().size.x
// WRONG (SFML 2)
getLocalBounds().width

// COLOR ALPHA TYPE — CORRECT (SFML 3 / C++17)
static_cast<uint8_t>(alpha)
// WRONG (SFML 2) — sf::Uint8 does not exist in SFML 3
static_cast<sf::Uint8>(alpha)

// sf::Text DEFAULT CONSTRUCTOR — does not exist in SFML 3
// Struct members of type sf::Text cannot be default-initialized.
// Use raw data (string, color, position) and construct sf::Text at render time.

// MOUSE POSITION
sf::Vector2i rawPos = sf::Mouse::getPosition(window);
sf::Vector2f worldPos = window.mapPixelToCoords(rawPos);

// HOVER DETECTION PATTERN
bool isHovered(const sf::RectangleShape& rect, const sf::Vector2f& mousePos) {
    return rect.getGlobalBounds().contains(mousePos);
}
```

---

## Engineering Rules

**Do not:**
- Rewrite working systems without a clear reason
- Break existing gameplay loops or save compatibility
- Introduce over-engineered abstractions
- Use hardcoded absolute asset paths (always relative)
- Edit backup files (`src_BACKUP/`, `data_BACKUP/`, `*_backup*`, `*_filtered*`)
- Make changes to more than 3 files at once without explicit user approval

**Always:**
- Read every relevant file BEFORE writing any code
- Make the minimal reliable change needed
- Explain root cause + why the fix works
- Keep UI logic separate from gameplay logic
- Produce complete, compileable code snippets
- Match existing code style
- Check whether `CharacterManager`, `CardManager`, `BattleSystem`, or `StoryManager` already has what you need before building something new

---

## Hard Code Rules

### Memory
- State-owned heap objects: use raw pointers cleaned up in destructor (existing pattern)
- State instances in Game: `std::unique_ptr` (existing pattern)
- Do not change the memory model of existing systems

### File I/O
- `SaveManager` writes JSON manually (hand-rolled). Do not switch to `nlohmann` for writes without updating ALL existing save/load pairs simultaneously.
- `nlohmann/json` is used for READING. Use `json::parse()` with try/catch.

### SaveManager Extension Rule
When adding new fields to `PlayerData`:
1. Add to struct
2. Add serialization line in `SaveManager::savePlayer()`
3. Add deserialization in `SaveManager::loadPlayer()`
Never remove existing fields. Old save files must still load without crashing.

### JSON Data Design
All new game content (achievements, milestones, quests, cards) must be JSON-driven. No hardcoded content arrays in `.cpp` files. Content lives in `data/`.

### Class Size Rule
If a `.cpp` file exceeds ~600 lines, extract a helper class. `TrainingState.cpp` is already large — new battle features should go into `BattleSystem`.

### Separation Rule
- UI rendering → state `render()` functions
- Game logic → `BattleSystem`, `CharacterManager`, `CardManager`, `StoryManager`
- Persistence → `SaveManager`
Do not merge these concerns.

---

## Forbidden Behaviors

These have caused build corruption before. Never do them:

- **Never use SFML 2.x syntax** — not even in comments as examples
- **Never rewrite a working system** — extend it
- **Never use `sf::Sprite sprite;` with no argument** — SFML 3 requires texture at construction
- **Never use `event.type ==`** — SFML 3 uses `event.getIf<>()`
- **Never output placeholder / stub implementations** — only real, compilable code
- **Never add a GameState enum value without updating all three switch blocks in Game.cpp**
- **Never remove fields from PlayerData** — breaks existing save files
- **Never use absolute file paths** — assets break when built to `build/`
- **Never produce a full file rewrite when only one function needs changing**
- **Never make changes to more than 3 files at once without explicit user approval**

---

## Pre-Output Checklist

Run this internally before every code response:

```
✓ Have I read the relevant file(s) before writing changes?
✓ Does this compile against SFML 3.0.2? (no SFML 2 syntax)
✓ If a new GameState was added, are all 3 switch blocks in Game.cpp updated?
✓ If PlayerData was extended, are both savePlayer() and loadPlayer() updated?
✓ Are all asset paths relative?
✓ Is JSON parsing in a named function with try/catch?
✓ Does this preserve all existing state transitions?
✓ Am I only changing what needs to change (not rewriting the file)?
✓ Does CharacterManager/CardManager/BattleSystem/StoryManager already have
   what I need before building something new?
```

---

## Debugging Protocol

1. Identify the symptom
2. Isolate the subsystem
3. Validate assumptions
4. Find root cause
5. Propose the minimal fix
6. Give verification steps

---

## Response Protocol

### For Bug Fixes
1. State: affected file + function name
2. State: root cause in one sentence
3. Show: only the changed function or block (not the full file)
4. State: how to verify the fix works

### For New Features
1. List all files that need changes
2. Implement in order: header → .cpp → Game.hpp/cpp wiring
3. Note any new data files or save format changes
4. State the testing procedure

### For Architecture Decisions
1. State the tradeoff explicitly
2. Default to the simpler option
3. Reference which existing system is being extended

### Output Format Rules
- Show diffs / changed blocks, not entire files (unless the file is new)
- Real, compilable C++ only — no pseudocode
- If uncertain about an SFML 3 API: say so, do not guess
- If a feature requires more than 3 files changed: ask before proceeding

---

## Design Philosophy

This is an **educational game first** — game feel, visual polish, and RPG immersion take priority over backend complexity or enterprise-style abstractions. The educational layer should feel naturally woven into combat and progression, not bolted on as a quiz.

Visual target: fantasy RPG × anime-inspired combat × cinematic particle-heavy feedback.
Inspirations: Persona, Slay the Spire, Honkai Star Rail, RPG Maker combat feel.

---

## Known Pitfalls (Do Not Repeat)

### 1. Question filtering is already done — do not re-implement it
`TrainingState::loadQuestions()` already builds the exact file path from grade + subject. Before touching question loading, read the existing code first.

### 2. `TrainingState` uses raw pointers for UI — this is intentional legacy, not a bug
`TrainingState.hpp` declares all UI widgets as raw `sf::Text*` / `sf::RectangleShape*` and allocates them with `new` in the constructor. **Do not refactor this to smart pointers unless explicitly asked.** The `cleanup()` method handles deletion; touching the ownership model risks double-free or dangling pointer bugs.

### 3. SFML 3 API differences from SFML 2 — always use v3 syntax
See the full SFML 3 Syntax Contract section above.

### 4. Grade string format must match exactly
Any spacing or capitalisation difference in grade strings returns `""` from `gradeToFolderPrefix()` and silently loads no questions. See the exact strings in the Grade → Folder Mapping section above.

### 5. Subject-to-filename mapping: only `Mathematics` is different
`subjectToFilename()` maps `"Mathematics"` → `"Math"`. All other subjects map to themselves.

### 6. Two subject fields exist — primary vs fallback
`Game` has both `game->selectedSubject` (set during subject selection UI) and `game->playerData.selectedSubject` (loaded from save). `loadQuestions()` uses `selectedSubject` first, falling back to `playerData.selectedSubject`. When setting subject elsewhere in the code, always set `game->selectedSubject`.

### 7. Backup files are dead code — never edit them
These files exist in the repo but are **not compiled** and must never be modified:
- `src/TrainingState_backup_filter.cpp`
- `src/TrainingState_filtered.cpp`
- `src/TrainingState_new_load.cpp`
- `src_BACKUP/` (entire directory)
- `data_BACKUP/` (entire directory)

### 8. Run the executable from the project root
All asset and data paths are relative. Running the binary from a different directory causes all file loads to silently fail.

### 9. The custom JSON parser is fragile — use it for question files only
`TrainingState.cpp` contains hand-rolled JSON helpers (`jsStr`, `jsArr`, `readJsonStr`). For any new JSON file format, prefer `nlohmann/json` (already a project dependency) rather than extending the hand-rolled parser.

### 10. CharacterManager, CardManager, and StoryManager are built but not wired
These classes are fully implemented and working. Before writing new systems, check if what you need already exists in one of these managers. The most common mistake is reimplementing something that already exists.

### 11. Adding a new GameState requires touching 5 locations
Enum in `Game.hpp` + 3 switch blocks in `Game.cpp` (`handleEvents`, `update`, `render`) + new `switchTo*()` method declaration in `Game.hpp` + its implementation in `Game.cpp`. All 5 must be done together or the game will crash or silently skip the new state.
