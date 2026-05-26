# The Rise of Luminary

A cinematic educational RPG / roguelike deck-builder built in **C++17** with **SFML 3.0.2**.

Learning is woven directly into combat — players answer subject questions during battles to attack, build combos, trigger criticals, and earn XP. The goal is to make education feel like gameplay, not a quiz.

---

## Tech Stack

| Tool | Version |
|---|---|
| C++ | 17 |
| SFML | 3.0.2 |
| Build | CMake |
| JSON | nlohmann/json |

---

## Game Flow

```
SPLASH → MAIN MENU → PROFILE SELECTION → REGISTRATION
       → CHARACTER SELECTION → TRAINING / BATTLE → STORY MODE
```

---

## Features

- **Question-driven combat** — answer correctly to attack; wrong answers let the enemy strike back
- **Combo & streak system** — consecutive correct answers multiply damage and trigger particle effects
- **5 subjects** — Mathematics, Science, History, Literature, Arts
- **5 grade levels** — Preschool, Elementary, Middle School, High School, College Prep
- **24 question files, 440+ questions** — automatically filtered by the player's selected subject and grade
- **Story mode** — 30-chapter narrative per subject with boss battles
- **Character selection** — multiple playable characters with unique stats
- **Profile system** — JSON save files with XP, streaks, and progress tracking
- **Visual feedback** — screen shake, particle bursts, animated HP bars, and elemental combat types
- **Cinematic intro** — video frame sequence on launch

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

---

## Building

### Prerequisites

- CMake 3.15+
- SFML 3.0.2
- A C++17-capable compiler (MSVC, GCC, Clang)

### Build Steps

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

Run the executable from the project root so relative asset paths resolve correctly.

---

## Key Source Files

| File | Role |
|---|---|
| `Game.cpp/hpp` | Main loop, state switching, shared player data |
| `TrainingState.cpp/hpp` | Core battle + question system |
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

## Design Philosophy

Educational game first — RPG immersion and game feel take priority over backend complexity. The educational layer should feel naturally woven into combat and progression, not bolted on as a quiz.

Visual target: fantasy RPG × anime-inspired combat × cinematic particle-heavy feedback.
Inspirations: Persona, Slay the Spire, Honkai Star Rail, RPG Maker combat feel.
