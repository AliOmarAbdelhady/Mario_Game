# Mario C++ Realistic Prototype

This project is a structured C++ foundation for a realistic, high-graphics Mario-style platformer using SFML.

## What Is Implemented

- Fixed timestep game loop (`120 Hz`) for stable gameplay physics.
- Mario-like movement with:
- acceleration/deceleration
- jump buffering
- coyote time
- variable jump height
- tile-based collision resolution
- Camera with smooth follow + look-ahead.
- Parallax sky/background layers.
- Particle effects for jump, landing, and run dust.
- Shader post-process pass for dynamic lighting + vignette.
- Clean module structure (`world`, `entities`, `render`, `core`).

## Project Structure

- `CMakeLists.txt`
- `src/main.cpp`
- `src/Game.hpp`
- `src/Game.cpp`
- `src/core/Config.hpp`
- `src/core/InputState.hpp`
- `src/world/TileMap.hpp`
- `src/world/TileMap.cpp`
- `src/entities/Player.hpp`
- `src/entities/Player.cpp`
- `src/entities/ParticleSystem.hpp`
- `src/entities/ParticleSystem.cpp`
- `src/render/CameraController.hpp`
- `src/render/CameraController.cpp`
- `src/render/ParallaxBackground.hpp`
- `src/render/ParallaxBackground.cpp`
- `src/render/LightingPass.hpp`
- `src/render/LightingPass.cpp`
- `assets/shaders/lighting.frag`
- `docs/REALISTIC_ROADMAP.md`

## Build

### 1) Install dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake libsfml-dev
```

### 2) Configure and build

```bash
/usr/bin/cmake -S . -B build
/usr/bin/cmake --build build -j
```

### 3) Run

```bash
./build/mario_game
```

## Controls

- `A / D` or `Left / Right`: move
- `Space / W / Up`: jump
- `Left Shift`: sprint

## Next Milestones

The full realistic production plan is in `docs/REALISTIC_ROADMAP.md`.
