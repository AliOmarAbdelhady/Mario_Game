# Realistic Mario-Style Game Plan (C++)

## Goal

Build a polished side-scroller with modern visuals, physically convincing movement, strong game feel, and production-grade architecture.

## Phase 1: Core Playable Vertical Slice (Now)

- Stable engine loop with fixed simulation step.
- Character controller with polished jump behavior and responsive controls.
- Collision against tile world.
- Camera follow with smoothing and look-ahead.
- Basic atmospheric visuals (parallax + particles + post process).

Exit criteria:
- Playable level from start to finish with no physics instability.

## Phase 2: Rendering Upgrade (High Graphics)

- PBR-inspired 2.5D art style:
- high-resolution character rig/sprites (8-direction body parts)
- normal maps for terrain and characters
- physically motivated lighting (key/fill/rim)
- shadow layers and ambient occlusion masks
- Multi-pass rendering:
- geometry pass
- light accumulation pass
- post processing (bloom, color grading, film grain)
- Weather/fog pipeline and time-of-day presets.

Exit criteria:
- One fully lit, art-complete biome with clear style direction.

## Phase 3: Animation and Realism

- Replace primitive player drawing with skeletal or segmented animation.
- Add state machine with blend trees:
- idle, walk, run, jump, fall, land, skid, wall slide
- Secondary motion and anticipation:
- squash/stretch curves
- procedural tilt and head stabilization
- animation events for dust/footstep sync.

Exit criteria:
- Movement readability remains high at all speeds and looks physically grounded.

## Phase 4: World Systems

- Data-driven level format (Tiled JSON or custom chunk format).
- Streaming large worlds by chunk.
- Interactive objects:
- moving platforms
- breakable blocks
- enemies with behavior trees or utility AI
- checkpoints and save/load.

Exit criteria:
- At least 3 connected levels, each with unique gameplay flow.

## Phase 5: Audio + Juiciness

- Layered adaptive music.
- Material-aware SFX (grass, wood, stone).
- Dynamic mixing with sidechain and ducking.
- Screen feedback:
- camera shake profiles
- hit-stop and slowdown
- subtle chromatic aberration for impacts.

Exit criteria:
- Player actions feel impactful with consistent audio/visual feedback.

## Phase 6: Production Quality

- Automated tests:
- collision/physics regression tests
- deterministic replay checks
- Performance budget targets:
- 120 simulation FPS
- 60/120 render FPS profiles depending on hardware tier
- Tooling:
- hot-reload for shaders/assets
- in-game debug overlays (collision, velocity, frametime)
- Build/release pipeline with reproducible packaging.

Exit criteria:
- Stable release candidate with CI passing and no critical gameplay bugs.

## Suggested Tech Stack

- Language: C++20
- Window/2D rendering: SFML (prototype), consider SDL2 + custom renderer for advanced pipeline
- Physics: custom deterministic platformer physics
- Data: JSON/TOML for tunable gameplay values
- Tooling: CMake + clang-tidy + sanitizers in debug builds

## Implementation Rules for Correctness

- Keep gameplay deterministic at fixed timestep.
- Separate simulation from rendering interpolation.
- Use data-driven tuning for movement constants.
- Keep collision response axis-separated and unit-tested.
- Never tie gameplay state updates to variable frame time.
- Keep render/post-process optional so gameplay still works on lower-end hardware.
