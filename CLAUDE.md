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
| Build | CMake |
| JSON | nlohmann/json |

Architecture: state-machine, data-driven, JSON content pipelines, OOP.

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

---

## Development Priorities

1. **Story integration** — dialogue cutscenes, chapter progression, boss battles
2. **Progression systems** — statistics screen, XP balancing, skill trees
3. **Reward systems** — card unlocks, character progression
4. **Polish** — animations, combat feedback, audio, UI

---

## Coding Standards

**Language:** C++17 — use STL containers, RAII, `std::unique_ptr`. No raw `new`/`delete`.

**Naming:**
- Classes: `PascalCase`
- Methods: `camelCase`
- Constants: `UPPER_CASE`

**SFML 3.0.2 — use v3 APIs only.**

```cpp
// Correct event handling (SFML 3)
if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) { ... }

// Correct vector
sf::Vector2f(100.f, 200.f)
```

Never use SFML 2-style APIs.

---

## Engineering Rules

**Do not:**
- Rewrite working systems without a clear reason
- Break existing gameplay loops or save compatibility
- Introduce over-engineered abstractions
- Use hardcoded absolute asset paths (always relative)
- Edit backup files (`src_BACKUP/`, `data_BACKUP/`, `*_backup*`, `*_filtered*`)

**Always:**
- Make the minimal reliable change needed
- Explain root cause + why the fix works
- Keep UI logic separate from gameplay logic
- Produce complete, compileable code snippets
- Match existing code style

---

## Debugging Protocol

1. Identify the symptom
2. Isolate the subsystem
3. Validate assumptions
4. Find root cause
5. Propose the minimal fix
6. Give verification steps

---

## Design Philosophy

This is an **educational game first** — game feel, visual polish, and RPG immersion take priority over backend complexity or enterprise-style abstractions. The educational layer should feel naturally woven into combat and progression, not bolted on as a quiz.

Visual target: fantasy RPG × anime-inspired combat × cinematic particle-heavy feedback.
Inspirations: Persona, Slay the Spire, Honkai Star Rail, RPG Maker combat feel.

---

## Known Pitfalls (Do Not Repeat)

### 1. Question filtering is already done — do not re-implement it
`TrainingState::loadQuestions()` already builds the exact file path from grade + subject. The "Current blocker" note that previously appeared in this file was **outdated**. Before touching question loading, read the existing code first.

### 2. `TrainingState` uses raw pointers for UI — this is intentional legacy, not a bug
`TrainingState.hpp` declares all UI widgets as raw `sf::Text*` / `sf::RectangleShape*` and allocates them with `new` in the constructor. This contradicts the "no raw new/delete" rule in the coding standards. **Do not refactor this to smart pointers unless explicitly asked.** The cleanup() method handles deletion; touching the ownership model risks double-free or dangling pointer bugs.

### 3. SFML 3 API differences from SFML 2 — always use v3 syntax
Common traps:
```cpp
// WRONG (SFML 2)
sf::Text text;
text.setFont(font);
text.setString("hello");
text.setCharacterSize(18);

// CORRECT (SFML 3)
sf::Text text(font, "hello", 18);

// WRONG (SFML 2)
if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)

// CORRECT (SFML 3)
if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    if (kp->code == sf::Keyboard::Key::Escape)

// WRONG — getLocalBounds().width (SFML 2)
// CORRECT — getLocalBounds().size.x (SFML 3)

// WRONG — setPosition(x, y) with two floats (SFML 2)
// CORRECT — setPosition(sf::Vector2f(x, y)) (SFML 3)
```

### 4. Grade string format must match exactly
`gradeToFolderPrefix()` in `TrainingState.cpp` matches these exact strings:
```
"Preschool"     → Class_Preschool/Preschool_
"Elementary"    → Class_Elementary/Elementary_
"Middle School" → Class_MiddleSchool/MiddleSchool_
"High School"   → Class_HighSchool/HighSchool_
"College Prep"  → Class_CollegePrep/CollegePrep_
```
Any spacing or capitalisation difference returns `""` and silently loads no questions.

### 5. Subject-to-filename mapping: only `Mathematics` is different
`subjectToFilename()` maps `"Mathematics"` → `"Math"`. All other subjects (`Science`, `History`, `Literature`, `Arts`) map to themselves. If you add a new subject, check whether its JSON file uses the full name or an abbreviation.

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
All asset and data paths are relative (e.g., `"assets/images/battlefield.png"`, `"data/questions/..."`). Running the binary from a different directory causes all file loads to silently fail.

### 9. The custom JSON parser is fragile — use it for question files only
`TrainingState.cpp` contains hand-rolled JSON helpers (`jsStr`, `jsArr`, `readJsonStr`). They work for the flat question format but are not general-purpose. For any new JSON file format, prefer `nlohmann/json` (already a project dependency) rather than extending the hand-rolled parser.
