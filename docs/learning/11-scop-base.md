# Build HumanGL on your scop project

[Guide index](README.md) · [Rendering setup](02-rendering-setup.md)

## The base and what was imported

The base is `/Users/ttokesi/project/scop`, commit `3073cf2` (`update readme`).
Its `src/`, `include/`, generated `glad.cpp`, and Makefile were copied into
HumanGL, with `models/cube.obj` and `textureSources/wood.bmp` for a working viewer
example. The source project itself was only inspected; it was not edited.

The separate C triangle starter was removed. Reusing scop means keeping C++20
and your existing classes rather than rebuilding the window/rendering layer in C.
The conceptual matrix and hierarchy exercises still apply; their conventions
are now aligned with scop's row-major storage.

Large model/texture collections, the demo GIF, and the scop subject PDF are not
required by this base. The standalone BMP test was not imported: it refers to a
missing `Window::getWindow()` accessor and an absent `wood_190S.bmp` asset, and
is not part of scop's Makefile. HumanGL follows the assignment supplied in this
chat, rather than inheriting OBJ-viewer requirements from the separate subject.

## Reuse map

| Component in this repository | What it already does | How HumanGL builds on it |
| --- | --- | --- |
| [Window](../../src/window/Window.cpp) | GLFW initialization, 4.1 core context, events, and window lifetime. | Retain it; expose/query actual framebuffer dimensions and guard minimized frames. |
| [ShaderProgram](../../src/shaders/ShaderProgram.cpp) and `src/shaderSources/` | Compile/link GLSL 410 and transform vertices with projection × view × model. | Keep the pipeline; add a plain body-color path that does not require a BMP. |
| [Render](../../src/render/Render.cpp) and [RenderDraw](../../src/render/RenderDraw.cpp) | Shared unit cube, one indexed draw per part, and separate frame preparation. | Traverse the body with `drawUnitCube(stack, color)` calls. |
| [MatrixTransform](../../src/matrixMath/MatrixTransform.cpp) and [MatrixStack](../../src/matrixMath/MatrixStack.hpp) | Custom transformations and a bounded value-copy stack. | Check the remaining Y-rotation convention; use the stack for joints. |
| [Transformation](../../src/scene/Transformation.cpp) | Builds one model transform for the viewer. | Use its root manipulation for the whole character; keep joint poses separate. |
| [Camera](../../src/scene/Camera.cpp) | Projection and resize aspect; view currently remains identity. | Implement a real view translation/look-at so the person can stand in world space. |
| [InputHandler](../../src/inputHandler/InputHandler.cpp) | Keyboard/mouse callbacks and access to scene state. | Retain callbacks; add animation mode, pause/reset, and joint demonstration controls. |
| [ControlsPanel](../../src/gui/ControlsPanel.cpp) | On-screen shortcuts, animation buttons, independent joint buttons, and view/size controls. | Shares commands and joint setters with keyboard input; captures panel input and reserves a separate 3D viewport. |
| [OBJ loader](../../src/modelLoader/RenderModelLoader.cpp), `src/texture/` | Existing viewer geometry and BMP handling. | Preserve the viewer mode; the procedural cube path needs no OBJ/BMP. |

The Makefile mentions GLM in the original project, but the inspected source does
not use it. The imported build removes those unused include flags. Your own
`MatrixTransform` remains the maths implementation.

## Keep one convention across CPU and shader

Scop stores matrices **row-major**: `matrix[row * 4 + column]`. Its shader uses
column-vector multiplication, and `RenderDraw.cpp` uploads with `GL_TRUE` to
transpose the row-major memory representation for OpenGL. That combination is
valid. Keep it consistently; do not switch just the upload flag or copy
column-major indices from another tutorial.

Translation occupies indices **3, 7, 11**. `A * B * p` still applies B before A.
The existing rotation methods accept **degrees** and convert to radians inside
their scalar trigonometric calculations. Preserve that public convention while
developing the body; animation phase and sine/cosine arguments remain radians.

The existing translation/rotation helpers edit selected entries rather than
generally concatenating transforms. Build each local transform from identity,
then use `MatrixTransform::multiply(parent, local, result)` to compose it. Do not
call these helpers directly on a rotated stack top and assume post-multiplication.

## Issues found before adding a skeleton

The following were found in the imported code. Projection/scaling, the shared
cube, matrix stack, and frame separation are now implemented. Remaining items
are identified below. This does not yet
establish correctness of every matrix operation or the future hierarchy.

1. **Perspective — fixed.** The old builder left `matrix[15]` at 1. It now
   replaces all 16 entries, sets that element to 0, and produces clip w=−z.
   Tests check exact entries, near/far depth, and perspective size with distance.
   Invalid/nonfinite frustum parameters throw without altering the output. The
   default model now starts at Z=−2, so the corrected camera can see the cube.
2. **Scaling — fixed.** The old implementation only multiplied diagonal entries.
   `scale(matrix, x, y, z)` now post-multiplies by local-axis scaling, including
   off-diagonal entries, while leaving translation unchanged. The existing
   uniform overload delegates to it. It works on identity as a scale builder,
   or on a composed matrix; tests cover rotation, translation, and separate axes.
3. **Y rotation uses the opposite sign from the guide's right-hand convention.**
   `rotateY(+90°)` maps +Z to −X, while the chosen right-hand convention expects
   +X. Standardize the signs, including input direction, before defining a gait.
4. **Matrix stack and limb chains — implemented.** `MatrixStack` copies complete
   row-major matrices, keeps a base identity, and rejects overflow/underflow
   without mutation. The `--cubes` demo draws independent siblings from a shared
   parent. `--limbs` (also `--arms` or `--arm`) reuses the two-part chain four
   times: two arms and two legs beneath them. Dimensions are shared; joint poses
   are independent. `Humanoid` now attaches the arm pair to
   the torso's sides, the leg pair underneath it, and the head on top, drawing
   all ten body parts.
5. **Unit cube — implemented.** The imported OBJ still spans ±1, but
   `makeUnitCubeMesh()` creates fixed ±0.5 geometry and 36 indices. Each renderer
   uploads that cube once. `drawUnitCube(stack, color)` reads the current matrix
   and issues exactly one indexed draw, without changing the stack or geometry.
6. **Frame/draw separation — implemented.** `beginFrame(camera)` clears and
   uploads the camera once. `drawUnitCube()` and `drawMesh()` draw individual
   objects. Viewer rotation/material blending now advance once in `main.cpp`,
   before matrix evaluation; the integer `1 / 60` gate has been removed.
7. **Renderer ownership — clarified.** `Render` borrows the shader program and
   owns its mesh buffers plus a 1×1 white fallback texture. Its idempotent
   `cleanUp()` releases those resources; the caller deletes the program afterward.
   The fallback keeps the shared shader's legacy sampler complete even without
   a BMP. Cleanup for loaded BMP texture objects remains a separate pending task.

The main loop now queries actual framebuffer pixel dimensions through `Window`,
updates viewport/aspect, and skips zero-size frames while waiting for events.

## Small import changes already made

- **Build:** executable name `humangl`, GLFW flags from `pkg-config` on both OSes,
  preserved warnings-as-errors, dependency tracking and debug flag, sequential
  clean rebuild, and a valgrind example using the included assets. No user
  `PKG_CONFIG_PATH` is overwritten.
- **`main.cpp`:** usage names the included assets; the timer starts after GLFW
  initialization; model/view/projection matrices are updated before upload; the
  shader program is bound before setting its sampler uniform.
- **OBJ face colors:** explicit float conversion for `RAND_MAX`, so current Apple
  Clang compiles the original random-color expression with `-Werror` enabled.
- **BMP header:** explicit `<cstdint>` and `std::uint16_t`/`std::uint32_t`, fixing
  missing type declarations with Linux's standard library without changing layout.

Those were the initial import changes. Subsequent corrections affect
`MatrixTransform`, `Camera`, and the default model translation. The next stage
added the shared cube, stack, frame/draw separation, and demo using those same
scop classes. Example assets and the original scop project remain unchanged.

## Build the project in this order

1. **Prove the inherited viewer works.** Build and run the included cube/BMP
   command in the root README. Locate context creation, the shader program,
   `MatrixTransform`, the uniform upload, and the single indexed draw.
2. **Make the maths and camera trustworthy.** Retain row-major storage and degree
   rotation APIs. Projection and axis scaling are implemented and tested; correct
   the remaining Y convention and extend the point checks. Move the camera to
   view the origin from positive Z; set the initial model/global scale to 1 for
   the future unit body.
3. **Inspect the implemented cube.** Read `UnitCube.cpp` and
   `Render::drawUnitCube()`. Run `./humangl --cubes` without any assets and trace
   how three transforms reuse one mesh.
4. **Inspect the frame boundary.** `beginFrame(camera)` clears once and uploads
   view/projection. Each part draw uploads its own model/color. Use
   `make test-render` to check draw counts, shared VAO, pixels, and stack state.
5. **Inspect the implemented limbs.** Read `scene/Arm.cpp` and run
   `./humangl --limbs`. The earlier `pairedModelMatrices()` and
   `limbModelMatrices()` demonstrate copying a shared pose. The current
   `Humanoid` evaluates each chain separately with its own `HumanoidPose` entry
   and shared dimensions. Hold L/R to select a side and L+K or R+T for lower
   segments. Use A/D for arms and Q/E for legs. Geometry scale stays separate
   from joint inheritance; moving a lower segment affects only that segment.
6. **Complete all ten parts.** Inspect `Humanoid::modelMatrices()`: its centered
   torso and dimension-derived shoulder/hip attachments connect the four chains.
   The head's base follows the torso's top. It returns ten models while keeping
   torso scale out of the head and limb frames. Ten body parts reach the cube
   drawing method once each; a parent joint must affect only its descendants.
7. **Inspect the C++ sequence player.** `animation/Motions.cpp` defines walking
   and jumping with `step()` and `to()` commands. C repeats the in-place walk,
   V plays the jump, and B stands still. `SequencePlayer::update()` interpolates
   angles and world-space height using elapsed seconds. Keyboard input and
   playback share the joint setters and hierarchy.
8. **Complete the defense checklist.** Check resizing, inheritance, stack balance,
   drawing calls, and both target platforms before adding bonuses.

The next stage is the full manual acceptance and defense check.
The body demo now includes a Dear ImGui controls panel. Its upstream source and
license live in `third_party/imgui`; the project's matrix and cube code remain
responsible for the 3D hierarchy. `make test-render` checks the panel separately
from the body's one-draw-per-part requirement.
The implemented `MatrixStack` sits beside `MatrixTransform`. Extend the
existing classes rather than introduce another window or rendering framework.

## Validation so far

- The copied base builds on macOS using Apple Clang 21, with `-Wall -Werror -Wextra`.
- A temporary hidden-window probe exercised the actual copied window, shaders,
  cube loader, renderer, texture loader, and matrices: OpenGL 4.1 Metal; 36 cube
  indices; a colored center pixel; `GL_NO_ERROR`. It positioned/scaled the cube
  explicitly and did not test every input path or the complete viewer loop.
- `make test` now checks corrected projection/scaling, rejected frustums, safe
  resize aspects, and the default cube's visibility. It needs no graphics context.
  The initial CPU probe also identified the still-pending Y-direction issue.
- On 7 November 2026, all 97 projection/scaling assertions passed on macOS and
  Ubuntu. Fresh hidden-window probes rendered the cube using the application's
  default transform on both platforms, with colored pixels and no OpenGL error.
- Linux compilation/rendering and manual interaction results are recorded in
  [the validation chapter](09-validation-and-defense.md). Old triangle-starter
  results do not establish correctness of this replacement base.
- The torso, head, two arms, and two identical two-part legs are implemented;
  C++ sequences provide walking in place, jumping, and standing still.

## Shared cube and stack checks

`make test` includes 97 projection/scaling assertions, 100 stack assertions, and
unit-cube extent/topology/winding checks. `make test-render` checks real pixels,
one 36-index call per cube, a reused VAO, no per-part geometry uploads or clears,
unchanged stack state, the legacy mesh color path, and borrowed shader ownership.
The graphical target uses a hidden window and requires a display session; it does
not automate keyboard/mouse interaction.

The final CPU and graphical targets passed on macOS/OpenGL 4.1 and
Ubuntu/Mesa/OpenGL 4.5 on 7 November 2026. The Mac sampler warning observed during
development was resolved by the renderer-owned white fallback texture; a later
check passed without that warning.

The limb tests now include 549 hierarchy assertions. Run `./humangl --limbs` to
explore shared joint controls, matching forward movement, length-dependent
elbow/knee placement, and configurable pair spacing and row gap. The render
test checks orange/blue pixels in all eight blocks, gaps between them, and
exactly eight draws for the limbs.

The aligned-arm checks verify equal segment orientations and fixed shoulder
spacing as the shared shoulder/elbow controls change. Positive flexion bends
both chains toward local +Z; no left-side angle inversion is applied.

The forward-facing version passed CPU and hidden-render checks on macOS and
Ubuntu on 8 November 2026, including all 420 arm assertions.

The subsequent copied-leg stage passed the build, all CPU suites (including
549 limb assertions), and hidden rendering on macOS/OpenGL 4.1. Its eight-part
render check verifies shared buffers, one draw per block, and parent preservation.
This stage has not yet been rerun on Linux or checked with physical input.

The torso stage passed the macOS build, CPU checks including 147 new
torso/attachment assertions, and the hidden OpenGL 4.1 render check. Its nine
blocks use nine draws with no geometry reupload. Torso resizing updates the
shoulder and hip attachments while preserving limb dimensions. This stage has
not yet been checked on Linux or with physical input.
