# The Rise of Luminary

**A cinematic educational RPG where knowledge is your weapon.**

Answer questions. Trigger combos. Deal damage. Level up.

The Rise of Luminary is a turn-based RPG / roguelike deck-builder built in **C++17** with **SFML 3.0.2**. Learning is woven directly into combat — answer educational questions correctly to attack enemies, build streaks, trigger critical hits, and earn XP. Wrong answers let the enemy fight back.

The goal: make education feel like a power fantasy, not a quiz.

---

## Gameplay

Each battle is driven by a question-and-answer loop:

- **Answer correctly** — your character attacks, dealing damage scaled to your stats and elemental affinity
- **Build a streak** — consecutive correct answers multiply your damage and trigger particle burst effects
- **Answer wrong** — the enemy counterattacks; your streak resets
- **Level up** — earn XP to improve stats, unlock character tiers (Common → Legendary), and progress through story chapters

Combat also features:

- **Elemental system** — 6 elements (Fire, Water, Lightning, Nature, Dark, Light) with damage multipliers
- **Screen shake and particles** — visual feedback on every hit, combo, and critical
- **Animated HP bars and battle log** — follow the fight in real time
- **Parallax-scrolling battle backgrounds** — cinematic depth during combat

---

## Subjects and Grade Levels

Questions are automatically filtered by the player's subject and grade from 24 question files (440+ questions total):

| | Preschool | Elementary | Middle School | High School | College Prep |
|---|---|---|---|---|---|
| Mathematics | ✓ | ✓ | ✓ | ✓ | ✓ |
| Science | ✓ | ✓ | ✓ | ✓ | ✓ |
| History | ✓ | ✓ | ✓ | ✓ | ✓ |
| Literature | ✓ | ✓ | ✓ | ✓ | ✓ |
| Arts | ✓ | ✓ | ✓ | ✓ | ✓ |

Each subject also has a **30-chapter story mode** with narrative cutscenes and boss battles.

---

## Game Flow

```
SPLASH VIDEO
    └── MAIN MENU
            ├── PROFILE SELECTION   (returning player)
            └── REGISTRATION        (new player)
                    └── CHARACTER SELECTION
                                └── TRAINING HUB
                                        ├── TRAINING BATTLE
                                        └── STORY MODE
```

---

## Tech Stack

| | |
|---|---|
| Language | C++17 |
| Graphics / Audio | SFML 3.0.2 |
| Build | CMake 3.10+ |
| JSON | nlohmann/json |
| Platform | Windows (MSVC / MinGW) |

---

## Building

### Prerequisites

- **CMake** 3.10 or newer
- **SFML 3.0.2** (headers + libraries)
- A **C++17** compiler — MSVC 2019+, GCC 9+, or Clang 10+

### Build

```bash
git clone <repo-url>
cd THE-RISE-OF-LUMINARY-main

mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The build system automatically copies `assets/` and `data/` into the output directory after each build.

### Running

**Run the executable from the project root.** All asset and data paths are relative — launching from another directory will silently break all file loads:

```bash
# From the project root directory
.\build\Release\LuminaryRPG.exe
```

---

## Project Structure

```
THE-RISE-OF-LUMINARY-main/
│
├── src/                            All C++ source
│   ├── Game.cpp / Game.hpp         Main loop, state machine, shared player data
│   ├── TrainingState.cpp/.hpp      Core battle + question system
│   ├── TrainingStoryState.cpp/.hpp Story mode battle integration
│   ├── BattleSystem.cpp/.hpp       Turn-based combat engine
│   ├── Character.cpp/.hpp          Player stats, XP, rarity progression
│   ├── Enemy.cpp/.hpp              Enemy generation and scaling
│   ├── ElementSystem.cpp/.hpp      Elemental type damage multipliers
│   ├── SaveManager.cpp/.hpp        JSON save/load for player profiles
│   ├── ParticleSystem.cpp/.hpp     Visual particle effects
│   ├── ScreenShake.cpp/.hpp        Camera shake on hits
│   ├── ParallaxBackground.cpp/.hpp Scrolling combat backgrounds
│   ├── Card.cpp/.hpp               Card mechanics
│   ├── StoryManager.hpp            Story JSON loading
│   └── ...                         Menu, registration, hub, lesson viewer states
│
├── assets/
│   ├── fonts/                      arial.ttf
│   ├── images/                     Character portraits, backgrounds, UI art
│   ├── sounds/                     Theme song (MPEG)
│   └── video/frames/               Intro animation — 80 PNG frames + audio
│
├── data/
│   ├── questions/
│   │   ├── Class_Preschool/        Preschool_{Subject}.json
│   │   ├── Class_Elementary/       Elementary_{Subject}.json
│   │   ├── Class_MiddleSchool/     MiddleSchool_{Subject}.json
│   │   ├── Class_HighSchool/       HighSchool_{Subject}.json
│   │   └── Class_CollegePrep/      CollegePrep_{Subject}.json
│   └── stories/
│       └── {Subject}_Stories.json  30-chapter narrative per subject
│
├── saves/                          Auto-generated JSON player save files
└── CMakeLists.txt
```

---

## Key Source Files

| File | Role |
|---|---|
| `Game.cpp/hpp` | Main loop, state switching, shared player data |
| `TrainingState.cpp/hpp` | Core battle + question system (primary) |
| `TrainingStoryState.cpp/hpp` | Story mode battle integration |
| `BattleSystem.cpp/hpp` | Turn-based combat logic and damage calculation |
| `Character.cpp/hpp` | Player character stats and progression |
| `Enemy.cpp/hpp` | Enemy generation |
| `SaveManager.cpp/hpp` | JSON save/load for profiles |
| `ParticleSystem.cpp/hpp` | Visual particle effects |
| `ScreenShake.cpp/hpp` | Camera shake feedback |
| `ElementSystem.cpp/hpp` | Elemental combat type bonuses |
| `StoryManager.hpp` | Story JSON loading |
| `ParallaxBackground.cpp/hpp` | Scrolling battle backgrounds |

---

## Save System

Player profiles are saved to `saves/` as human-readable JSON files. Each profile tracks:

- Player name and selected character
- Grade level and subject
- XP, level, and stat progression
- Streak history and battle stats
- Story chapter progress

---

## Design Philosophy

**Educational game first.** RPG immersion, game feel, and visual polish take priority over backend complexity. The educational layer should feel naturally woven into combat and progression — not bolted on as a quiz.

Visual target: fantasy RPG × anime-inspired combat × cinematic particle-heavy feedback.

Inspirations: *Persona*, *Slay the Spire*, *Honkai: Star Rail*, RPG Maker combat feel.

---

## Roadmap

- [ ] Full story mode — dialogue cutscenes, chapter-by-chapter boss progression
- [ ] Skill trees and card unlock rewards
- [ ] Statistics screen with performance tracking
- [ ] XP curve balancing across grade levels
- [ ] Per-element and per-action sound effects
- [ ] Boss battle visual sequences

---

## License

Personal educational game project by Ishan. All rights reserved.
