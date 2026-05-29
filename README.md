<div align="center">

<img src="https://capsule-render.vercel.app/api?type=waving&color=gradient&customColorList=6,11,20&height=230&section=header&text=The%20Rise%20of%20Luminary&fontSize=54&fontColor=fff&animation=twinkling&fontAlignY=40&desc=A%20Cinematic%20Educational%20RPG&descAlignY=60&descFontSize=20" width="100%"/>

[![Typing SVG](https://readme-typing-svg.demolab.com?font=Cinzel+Decorative&size=17&pause=1200&color=FFD700&center=true&vCenter=true&width=650&lines=Answer+questions.+Deal+damage.+Level+up.;Knowledge+is+your+weapon.;Fantasy+RPG+%C3%97+Anime+Combat+%C3%97+Education;Built+in+C%2B%2B17+with+SFML+3.0.2;30+story+chapters+%C3%97+5+subjects+%C3%97+440%2B+questions)](https://git.io/typing-svg)

<br/>

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![SFML](https://img.shields.io/badge/SFML-3.0.2-8CC445?style=for-the-badge&logo=sfml&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.10%2B-064F8C?style=for-the-badge&logo=cmake&logoColor=white)
![JSON](https://img.shields.io/badge/JSON-nlohmann%2Fjson-F7C948?style=for-the-badge&logo=json&logoColor=black)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)

</div>

---

## What is The Rise of Luminary?

A **turn-based educational RPG / roguelike deck-builder** built in C++17 with SFML 3.0.2.

Learning is fused directly into combat. Answer real educational questions to attack, build combo streaks, trigger critical hits, earn XP, and advance through story chapters. The design goal is singular: **make education feel like a power fantasy, not a quiz.**

> _Fantasy RPG × Anime-inspired combat × Cinematic particle feedback_
>
> _Inspired by: Persona · Slay the Spire · Honkai: Star Rail · RPG Maker_

---

## ⚔️ Gameplay Loop

```
[ QUESTION APPEARS ]
        │
        ├─ Correct Answer ──► You attack!  Damage = stats × elemental affinity
        │                          └─ Streak builds ──► Combo multiplier activates
        │                                                    └─ CRIT? ──► Screen shake + particle burst!
        │
        └─ Wrong Answer ──► Enemy counterattacks  ·  Streak resets
```

Every correct answer in a row multiplies your damage. Chain enough hits and the screen erupts.

### Combat Systems

| System | Description |
|:---|:---|
| ⚡ Elemental Affinity | 6 elements — Fire, Water, Lightning, Nature, Dark, Light — each with damage multipliers |
| 💥 Combo & Streak | Consecutive correct answers multiply damage exponentially |
| 🎇 Particle System | Visual burst effects on every hit, combo, and critical |
| 📳 Screen Shake | Camera shake tied to damage magnitude |
| 📊 Animated HP Bars | Real-time health display with battle log |
| 🌄 Parallax Backgrounds | Scrolling layered scenes during combat |
| 📖 Story Mode | 30-chapter narrative per subject, with boss battles |

---

## 📚 Subjects × Grade Levels

**440+ questions across 24 JSON files**, auto-filtered at runtime by the player's grade and subject — no manual selection needed.

|  | Preschool | Elementary | Middle School | High School | College Prep |
|:---|:---:|:---:|:---:|:---:|:---:|
| ➕ Mathematics | ✅ | ✅ | ✅ | ✅ | ✅ |
| 🔬 Science      | ✅ | ✅ | ✅ | ✅ | ✅ |
| 🏛️ History      | ✅ | ✅ | ✅ | ✅ | ✅ |
| 📖 Literature   | ✅ | ✅ | ✅ | ✅ | ✅ |
| 🎨 Arts         | ✅ | ✅ | ✅ | ✅ | ✅ |

Each subject ships with a **30-chapter story mode** — narrative cutscenes, chapter bosses, and escalating difficulty.

---

## 🗺️ Game Flow

```
┌──────────────────────────────────────────────────┐
│            SPLASH VIDEO (80-frame intro)          │
└─────────────────────┬────────────────────────────┘
                      │
             ┌────────▼────────┐
             │    MAIN MENU    │
             └────────┬────────┘
              ┌───────┴────────┐
   ┌──────────▼───────┐  ┌────▼──────────────┐
   │  PROFILE SELECT  │  │   REGISTRATION    │
   │  (returning)     │  │   (new player)    │
   └──────────┬───────┘  └────────┬──────────┘
              └──────────┬────────┘
                ┌────────▼────────┐
                │CHARACTER SELECT │
                └────────┬────────┘
                  ┌──────▼──────┐
                  │TRAINING HUB │
                  └──────┬──────┘
            ┌────────────┴─────────────┐
    ┌────────▼───────┐        ┌────────▼──────┐
    │ LESSON VIEWER  │        │  STORY MODE   │
    └────────┬───────┘        └───────────────┘
    ┌────────▼───────┐
    │  BATTLE STATE  │  ◄── Core gameplay loop
    └────────────────┘
```

---

## 🔧 Building

### Prerequisites

- **CMake** 3.10 or newer
- **SFML 3.0.2** (headers + libraries)
- A **C++17** compiler — MSVC 2019+, GCC 9+, or Clang 10+

### Build Steps

```bash
git clone <repo-url>
cd THE-RISE-OF-LUMINARY-main

mkdir build && cd build
cmake ..
cmake --build . --config Release
```

> The CMake build automatically copies `assets/` and `data/` into the output directory after each build.

### Running

**Always launch from the project root.** All asset and data paths are relative — running from a different directory will silently fail all file loads.

```bash
.\build\Release\LuminaryRPG.exe
```

---

## 📁 Project Structure

<details>
<summary><b>Click to expand</b></summary>

```
THE-RISE-OF-LUMINARY-main/
│
├── src/
│   ├── Game.cpp / Game.hpp                Main loop, state machine, shared player data
│   ├── TrainingState.cpp / .hpp           Core battle + question system
│   ├── TrainingStoryState.cpp / .hpp      Story mode battle integration
│   ├── BattleSystem.cpp / .hpp            Turn-based combat engine
│   ├── Character.cpp / .hpp               Player stats, XP, rarity progression
│   ├── Enemy.cpp / .hpp                   Enemy generation and scaling
│   ├── ElementSystem.cpp / .hpp           Elemental damage type multipliers
│   ├── SaveManager.cpp / .hpp             JSON save/load for player profiles
│   ├── ParticleSystem.cpp / .hpp          Visual particle burst effects
│   ├── ScreenShake.cpp / .hpp             Camera shake on hits
│   ├── ParallaxBackground.cpp / .hpp      Scrolling layered combat backgrounds
│   ├── Card.cpp / .hpp                    Card mechanics
│   ├── Button.cpp / .hpp                  Animated UI button system
│   ├── StoryManager.hpp / .cpp            Story JSON loading (30 chapters × 5 subjects)
│   ├── AchievementManager.cpp / .hpp      10-achievement system, per-user save files
│   ├── StatisticsState.cpp / .hpp         Performance tracking and daily quest display
│   ├── SettingsState.cpp / .hpp           Volume and fullscreen settings
│   ├── TrainingHubState.cpp / .hpp        Chapter / lesson selection hub
│   ├── LessonViewerState.cpp / .hpp       In-game lesson viewer UI
│   └── ...                                Menu, registration, character select states
│
├── assets/
│   ├── fonts/                             arial.ttf
│   ├── images/                            Character portraits, backgrounds, UI art
│   ├── sounds/                            Theme song (MPEG)
│   └── video/frames/                      Intro animation — 80 PNG frames + WAV audio
│
├── data/
│   ├── questions/
│   │   ├── Class_Preschool/               Preschool_{Subject}.json × 5
│   │   ├── Class_Elementary/              Elementary_{Subject}.json × 5
│   │   ├── Class_MiddleSchool/            MiddleSchool_{Subject}.json × 5
│   │   ├── Class_HighSchool/              HighSchool_{Subject}.json × 5
│   │   └── Class_CollegePrep/             CollegePrep_{Subject}.json × 5
│   └── stories/
│       └── {Subject}_Stories.json         30 chapters × 5 subjects
│
├── saves/                                 Auto-generated JSON player profiles
└── CMakeLists.txt
```

</details>

---

## 🗂️ Key Source Files

| File | Role |
|:---|:---|
| `Game.cpp / hpp` | Main loop, state switching, shared player data |
| `TrainingState.cpp / hpp` | Core battle + question system |
| `BattleSystem.cpp / hpp` | Combat engine, damage calculation |
| `Character.cpp / hpp` | Player stats, XP, rarity tier system |
| `ElementSystem.cpp / hpp` | Elemental damage type multipliers |
| `ParticleSystem.cpp / hpp` | Visual particle burst effects |
| `ScreenShake.cpp / hpp` | Camera shake on damage |
| `Button.cpp / hpp` | Animated UI buttons with full state machine |
| `TrainingHubState.cpp / hpp` | Chapter / lesson selection hub |
| `LessonViewerState.cpp / hpp` | In-game educational content viewer |
| `AchievementManager.cpp / hpp` | 10-achievement system with per-user persistence |
| `StatisticsState.cpp / hpp` | Performance tracking and daily quest display |
| `SettingsState.cpp / hpp` | Music/SFX volume and fullscreen settings |
| `SaveManager.cpp / hpp` | JSON save / load for player profiles |
| `ParallaxBackground.cpp / hpp` | Scrolling layered combat backgrounds |

---

## 💾 Save System

Player profiles live in `saves/` as human-readable JSON files. Each profile tracks:

- Player name, grade level, and selected subject
- Selected character and unlock status
- XP, level, and derived combat stats
- Streak history and battle performance
- Story chapter progress per subject

---

## 🗺️ Roadmap

- [x] Full story mode — 7-screen flow: intro → concept → quick-check → battle → results
- [x] Card unlock reward system — streak milestones 3/5/7 award cards in battle
- [x] Statistics screen — per-subject accuracy, daily quest progress, strongest/weakest subject
- [x] Achievement system — 10 achievements loaded from JSON, per-user save files
- [x] Settings menu — music/SFX volume, fullscreen toggle, persisted to JSON
- [x] Survival mode — lives system; wrong answers cost a life, 0 = game over
- [x] Endless mode — question pool reshuffles and loops instead of ending
- [x] Adaptive difficulty — adjusts grade level up/down based on subject accuracy
- [x] Daily quests — 3 daily targets tracked and displayed on statistics screen
- [x] Floating combat text — XP gain, wrong answer, level-up popups in battle
- [x] Screen fade transitions — cinematic fades between all menu options
- [ ] Per-element and per-action sound effects
- [ ] Boss battle visual sequences and special animations
- [ ] XP curve balancing across all 5 grade levels

---

## 👥 Credits

### Team

| | Name | Role |
|:---:|:---|:---|
| 👑 | **Ishan** | **Team Lead** — project architecture, initial codebase, game systems design |
| 🛠️ | **Jishnu Kaushik** | **Developer** — feature engineering, UI overhauls, systems implementation |
| 🎮 | **Yuvraj Chillar** | Team Member |
| 🎮 | **Aditya Sain** | Team Member |
| 🎮 | **Jatin Bhadwaj** | Team Member |

---

### Jishnu Kaushik — Implemented Changes

<details>
<summary><b>Full breakdown of features built by Jishnu Kaushik</b></summary>

<br/>

**⚔️ Training State Overhaul**
- Complete rewrite and expansion of `TrainingState.cpp` (550+ insertions) — restructured the core battle loop, question flow, and entire UI rendering pipeline
- Reorganised `TrainingState.hpp` — cleaned up member variables, added new UI element declarations for improved scalability

**🖥️ Battle HUD Layout System**
- Replaced 110+ hardcoded pixel positions with a centralised `Layout` namespace of named constants
- Covers every HUD component — player panel, enemy panel, score, question box, answer grid, feedback strip, mastery bar, bottom nav
- Layout tuning is now a single-location change instead of hunting through raw coordinates

**🔘 Button Class Upgrade**
- Designed and implemented a full state machine inside `Button` — pressed, disabled, hover, and idle states with clean transitions
- Added smooth **colour interpolation** between states for a polished, animated feel
- Enhanced callback system and input handling reliability

**🎭 Character Selection UI Revamp**
- Replaced static background with a scaled dynamic background (`characterselect.png`)
- Rebuilt character card rendering — textures, sprites, and outline boxes built dynamically in `updateDisplay()`
- Improved memory safety: vectors properly cleared, textures and sprites correctly owned and deleted
- Fixed nav-key input (highlight colours update correctly), removed broken numeric shortcuts

**🏛️ Training Hub & Lesson Viewer States**
- Built `TrainingHubState` from scratch — the chapter/lesson selection screen between the menu and combat
- Built `LessonViewerState` from scratch (260 lines) — full in-game lesson viewer with its own rendering pipeline and input handling
- Wired both into `Game.cpp` / `Game.hpp` — state transitions, lifecycle methods, and CMakeLists.txt build integration

**🏆 Achievement System**
- Built `AchievementManager` — loads 10 achievements from `data/achievements.json` with hardcoded fallback
- Per-user save files at `saves/{username}_achievements.json`; checks wired in battle and story states

**📊 Statistics Screen**
- Built `StatisticsState` — per-subject accuracy, strongest/weakest subject detection, daily quest progress display
- Wired into state machine as menu option 5 (STATISTICS)

**⚙️ Settings Menu**
- Built `SettingsState` — music volume, SFX volume (±5 per key, visual bar), fullscreen toggle
- Persists to `saves/settings.json`; wired as menu option 6 (SETTINGS)

**🎖️ Survival & Endless Modes**
- Survival mode: wrong answers cost a life; 0 lives triggers GAME OVER and returns to menu
- Endless mode: question pool reshuffles and loops instead of ending on completion

**🎯 Adaptive Difficulty**
- After grade selection, calculates subject accuracy from `topicAttempts`/`topicCorrect` (requires ≥10 attempts)
- accuracy < 50%: tries one grade below; accuracy > 80%: tries one grade above; falls back if file missing

**📅 Daily Quests & Topic Tracking**
- Three daily targets (10 questions, 1 story chapter, 2-day streak) tracked and reset on new calendar day
- Per-subject `topicAttempts`/`topicCorrect` maps serialized in SaveManager for adaptive difficulty and statistics

**✨ Polish — Fade Transitions & Floating Text**
- Screen fade transitions added for all 5 menu options via `startFade()` lambda in Game
- `FloatingText` added to `ParticleSystem` — green XP gain, red WRONG!, gold LEVEL UP! and achievement popups

**🧹 Codebase Cleanup & Documentation**
- Removed stale test data (`sample_questions.json`) superseded by the 24 real question files
- Tightened `.gitignore` — backup patterns scoped to `src/`, sample file explicitly excluded
- Authored the project `README.md` and clarified `CLAUDE.md` (removed outdated blocker note on question filtering)

</details>

---

<div align="center">

<img src="https://capsule-render.vercel.app/api?type=waving&color=gradient&customColorList=6,11,20&height=120&section=footer" width="100%"/>

_Personal educational game project — All rights reserved_

</div>
