# Tetris

A console-based Tetris game written in C++ for a basic programming course. The project uses functions, loops, arrays, structs, user input, and file handling to build a playable game.

## Features

- Move and rotate falling blocks
- Clear completed rows to increase your score
- Choose the board size and difficulty
- Preview the next block
- Save and load a game
- View separate leaderboards for easy and hard modes
- Try manual block selection and an experimental player-versus-player mode

## Requirements

- Windows
- A C++17 compiler
- CMake 3.25 or later

The game uses Windows console functions, so it is designed to run on Windows.

## Build and Run

Clone the repository and build it with CMake:

```bash
git clone https://github.com/parhammm13/Tetris.git
cd Tetris
cmake -S . -B build
cmake --build build --config Debug
```

With Visual Studio, run:

```powershell
.\build\Debug\untitled9.exe
```

The executable may instead be at `build/untitled9.exe`, depending on the CMake generator.

## Menu

| Option | Action |
| --- | --- |
| 1 | Start a new game |
| 2 | Load a saved game |
| 3 | Show instructions |
| 4 | Show the leaderboard |
| 5 | Start player-versus-player mode (experimental) |
| 6 | Play with manually selected blocks |
| 7 | Exit |

## Controls

| Key | Action |
| --- | --- |
| `A` | Move left |
| `D` | Move right |
| `S` | Move down |
| `W` | Rotate the block |
| `P` | Pause or resume |
| `C` | Save and leave the current game |
| `E` | Exit the current game |
| `R` | Restart the game |

In manual block mode, keys `0` through `6` select the next block type.

## How to Play

Arrange falling blocks to fill horizontal rows. A completed row disappears and adds to your score. The game ends when a new block cannot enter the board.

At the start of a game, enter a board width and height, choose difficulty `1` (easy) or `2` (hard), and enter your name. Hard mode makes the blocks fall faster.

## Project Files

- `main.cpp` — Game logic, menu, controls, display, saving, and leaderboard
- `CMakeLists.txt` — CMake build configuration

## About

Created as a project for a basic programming course.
