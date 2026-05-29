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

### BUG-001 [CRITICAL] — Subject + Grade Filtering Not Working
- **File:** `TrainingState.cpp` → `loadQuestions()`
- **Root cause:** Function iterates ALL `Class_*` folders and loads ALL JSON files, ignoring `game->selectedSubject` and `game->playerData.gradeLevel`
- **Fix:** Load only the single file matching grade + subject instead of full directory scan
- **Required data:** `game->selectedSubject` + `game->playerData.gradeLevel` → map to path
- **Status:** Open

### BUG-002 [CRITICAL] — Story Mode Not Accessible
- **Files:** `Game.hpp` (missing STORY_MODE enum value), `MenuState.cpp` (no menu option), `TrainingStoryState.cpp` (placeholder only — no chapter display)
- **Root cause:** `STORY_MODE` never added to `GameState` enum; no `switchToStoryMode()` method exists; `TrainingStoryState` has stub implementations
- **Fix order:** `Game.hpp` enum → `Game.cpp` switch blocks → `Game::switchToStoryMode()` → `MenuState` option → `TrainingStoryState` implementation using `StoryManager`
- **Status:** Open

### BUG-003 [MEDIUM] — XP Awarded But Not Persisted
- **File:** `TrainingState.cpp` → `checkAnswer()`
- **Root cause:** `Character::addXP()` exists and `CharacterManager::saveProgress()` exists, but neither is called after a correct answer in battle
- **Fix:** Call `charManager.addXP(xpAmount)` on correct answer, then `charManager.saveProgress()`
- **Status:** Open

### BUG-004 [MEDIUM] — CardManager Initialized But Never Used in Battle
- **File:** `TrainingState.cpp`
- **Root cause:** `CardManager` class is complete with `playCard()`, `drawInitialHand()` etc. but no instance exists in `TrainingState`; cards are never drawn or awarded
- **Fix:** Add `CardManager cardManager` to `TrainingState`, call `drawInitialHand()` on battle start, award cards on correct streaks
- **Status:** Open

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

### PHASE 1 — Wire Existing Systems Into Battle

**1.1 XP + Character Progression (BUG-003)**
- `CharacterManager` already has `addXP()`, `levelUp()`, `saveProgress()`
- Wire into `TrainingState::checkAnswer()` for correct answers
- Wire into battle victory for enemy defeat bonus using `Enemy::xpReward`
- Display level-up notification in battle log

**1.2 Lite Card Integration (BUG-004)**
- `CardManager` is already built. Wire it in.
- `TrainingState` gets a `CardManager cardManager` member
- On battle start: `cardManager.drawInitialHand()`
- On correct answer streak (3, 5, 7): `cardManager.addCardToDeck(randomCardId)` and show "Card Earned!" feedback
- Pre-battle screen (new state or overlay): pick 3 cards from owned deck that apply passive bonuses

**1.3 Mouse Support (UI-wide)**
- Every state with buttons/options needs mouse hover + click support
- `sf::Mouse::getPosition(window)` → `window.mapPixelToCoords()` → `bounds.contains()`
- Hover: highlight the option box; Click: same action as keyboard selection
- Keyboard and mouse must coexist — do not remove keyboard input

---

### PHASE 2 — Educational Features

**2.1 Weak Topic Tracking**
Add to `PlayerData`:
```cpp
std::map<std::string, int> topicAttempts;   // topic → total attempts
std::map<std::string, int> topicCorrect;    // topic → correct answers
```
On wrong answer: `topicAttempts[question.subject]++`
On correct answer: both maps increment. Save in `SaveManager`.

**2.2 Adaptive Difficulty**
After loading questions, calculate player accuracy per subject from `PlayerData` maps.
- If accuracy < 50%: weight question pool toward easier questions
- If accuracy > 80%: prefer questions from the next grade folder
JSON-driven — no hardcoded difficulty values.

**2.3 Statistics Screen**
New state: `StatisticsState`. Display using existing `PlayerData` fields:
- Overall accuracy: `correctAnswers / totalQuestionsAnswered * 100`
- Best streak, current streak, daily minutes played
- Per-subject accuracy from `topicCorrect`/`topicAttempts` maps
- Strongest/weakest subject, completed chapters list

---

### PHASE 3 — Progression + Retention

**3.1 Achievements**
JSON-tracked milestones. New file: `saves/{username}_achievements.json`
```json
[
  { "id": "first_win",     "name": "First Victory",   "unlocked": false },
  { "id": "streak_10",     "name": "On Fire",         "unlocked": false },
  { "id": "questions_100", "name": "Century Scholar", "unlocked": false },
  { "id": "all_chapters",  "name": "Story Complete",  "unlocked": false }
]
```
Check and unlock in `TrainingState` and `TrainingStoryState`. Show unlock popup via `ParticleSystem` + text overlay.

**3.2 Daily Quests**
Use `dailyMinutes` and `currentStreak` in `PlayerData`. Three daily targets (reset on date change using `lastPlayedDate`):
- Answer N questions today
- Maintain N-day login streak
- Complete one story chapter

**3.3 Character Milestone Upgrades**
Three milestone levels per character: 5 / 10 / 15. JSON-driven passive bonus per milestone.
Extend `characters.json` format:
```json
{
  "id": 1,
  "milestones": [
    { "level": 5,  "stat": "attack",    "bonus": 5  },
    { "level": 10, "stat": "defense",   "bonus": 3  },
    { "level": 15, "stat": "maxHealth", "bonus": 20 }
  ]
}
```
Apply in `Character::levelUp()`.

---

### PHASE 4 — Polish

**4.1 Screen Fade Transitions**
Add to `Game` class:
```cpp
sf::RectangleShape fadeOverlay;   // full-screen black rect
float fadeAlpha;
bool fadingOut, fadingIn;
```
On state switch: fade to black → switch state → fade from black. ~0.3s per direction. No new state needed — runs in `Game::update()`.

**4.2 Floating Combat Text**
Extend `ParticleSystem` (already exists) with:
`spawnFloatingText(sf::Vector2f pos, std::string text, sf::Color color)`
Spawn on: correct answer (green "+10"), wrong answer (red "WRONG"), level up (gold "LEVEL UP!").

**4.3 Survival Mode**
Same as TRAINING but with `lives = 3`. Toggle via `bool survivalMode` flag in `TrainingState` — no new state needed. Wrong answer: `lives--`; show life indicator in UI. `lives == 0`: game over screen.

**4.4 Endless Mode**
Remove the `questions.resize(50)` cap in `loadQuestions()`. When `currentQuestionIndex >= questions.size()`: reshuffle and loop. Track total answered across loops in stats.

**4.5 Settings Menu**
New state: `SettingsState`. Controls: music volume (slider 0–100), SFX volume (slider 0–100), fullscreen toggle. Save to `saves/settings.json`. Load on game start in `Game::initWindow()`.

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
