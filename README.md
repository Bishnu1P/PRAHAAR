# PRAHAAR — A 2D Survival Game

C++17 / SFML 2.6.1 survivor-like game. See `docs/design-doc.md` for core
stats, enemy types, weapons, and progression formulas.

## Requirements (Windows)

- **CMake** ≥ 3.16 — https://cmake.org/download/ (check "Add to PATH" during install)
- **A C++ compiler**: either
  - Visual Studio 2022 (Community edition, free) with the "Desktop development with C++" workload, **or**
  - MinGW-w64 (if you prefer g++)
- **Git** — https://git-scm.com/download/win

You do **not** need to manually download or install SFML — CMake's
`FetchContent` pulls and builds it automatically on first build. This
does mean the first build will take a few minutes and needs an internet
connection; subsequent builds are fast.

## Building

```powershell
git clone <your-repo-url>
cd PRAHAAR
cmake -B build -S .
cmake --build build --config Debug
```

The executable and `assets/` folder will be copied into `build/Debug/`
(or `build/` depending on generator). Run `PRAHAAR.exe` from there.

## Project structure

```
PRAHAAR/
├── CMakeLists.txt      # Build config (SFML fetched automatically)
├── src/                # All .cpp files — add new ones here, CMake picks them up
├── include/            # Header files (.h/.hpp)
├── assets/
│   ├── sprites/
│   ├── sounds/
│   └── fonts/
├── docs/
│   └── design-doc.md   # Core stats, enemy types, weapons, progression — READ THIS FIRST
└── .gitignore
```

## Step 1 checklist (Setup & Design Finalization)

- [ ] Everyone clones the repo and successfully builds the hello-window app
- [ ] Everyone can see a green circle on a dark background when running `PRAHAAR.exe`
- [ ] Team reviews and edits `docs/design-doc.md` together — resolve the open questions at the bottom
- [ ] Push design doc changes before moving to Step 2 (Core Engine & Game Loop)
