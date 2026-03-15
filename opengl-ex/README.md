# Refactor package for the ImGui/OpenGL demo

This package splits the monolithic `main.cpp` into focused modules:

- `math3d.*`  
  Small math layer for vectors, matrices, transforms, and `look_at()`.

- `orbit_camera.*`  
  The orbit camera and camera manipulation logic.

- `picking.*`  
  Mouse ray generation and cube face picking.

- `app_state.h`  
  Central app state shared by UI, renderer, and automation.

- `automation.*`  
  Record/replay support, command parsing, playback stepping.

- `cube_mesh.*`  
  The current demo geometry. This is the right place to grow into a scene/mesh module later.

- `viewport_renderer.*`  
  FBO management and scene rendering into the viewport texture.

- `ui_panels.*`  
  Current ImGui windows. This is the pattern to follow when you grow to dozens or hundreds of dialogs.

- `main_refactored.cpp`  
  A thin composition root that wires modules together.

## Why this structure scales better

Your current single-file version mixes all of these concerns in one place:

- math
- camera
- picking
- automation
- OpenGL resources
- FBO lifecycle
- UI
- app orchestration

That works for a prototype, but it becomes hard to extend once you add more geometry types, more tools, and many dialogs.

This refactor separates by responsibility:

- **state** lives in `AppState`
- **rendering** lives in `ViewportRenderer`
- **interaction/math** lives in `OrbitCamera` and `picking`
- **QA/dev automation** lives in `automation`
- **ImGui windows** live in `ui_panels`
- **startup/wiring** lives in `main_refactored.cpp`

## Suggested next steps

1. Keep `AppState` small and semantic.
2. Add a `Scene` module once you move beyond a single cube.
3. Split the current Scene Controls window into smaller panels as features grow.
4. Introduce a command bus or tool system when you add editing operations.
5. Move constants like FOV, pan speed, and highlight colors into a config struct.
