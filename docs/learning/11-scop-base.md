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
| [Render](../../src/render/Render.cpp) and [RenderDraw](../../src/render/RenderDraw.cpp) | Upload an interleaved mesh and prepare one indexed draw. | Separate frame preparation from drawing an individual part. |
| [MatrixTransform](../../src/matrixMath/MatrixTransform.cpp) | Identity, multiplication, translation, rotation, corrected uniform/nonuniform scaling and perspective. | Check the remaining Y-rotation convention; add a value-copy matrix stack. |
| [Transformation](../../src/scene/Transformation.cpp) | Builds one model transform for the viewer. | Use its root manipulation for the whole character; keep joint poses separate. |
| [Camera](../../src/scene/Camera.cpp) | Projection and resize aspect; view currently remains identity. | Implement a real view translation/look-at so the person can stand in world space. |
| [InputHandler](../../src/inputHandler/InputHandler.cpp) | Keyboard/mouse callbacks and access to scene state. | Retain callbacks; add animation mode, pause/reset, and joint demonstration controls. |
| [OBJ loader](../../src/modelLoader/RenderModelLoader.cpp), `src/texture/` | Existing viewer geometry and BMP handling. | Keep as reference; body geometry should use a guaranteed unit cube. |

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

The following were found in the imported code. Projection/scaling are now fixed
and covered by `make test`; the other items remain pending. This does not yet
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
4. **No matrix stack or joint hierarchy exists.** The current program owns one
   model matrix. Mouse rotation and automatic spinning move the entire OBJ.
5. **The example cube is 2×2×2.** Its coordinates are ±1, and the loader centers
   geometry without normalizing size. Using that asset unchanged as the required
   unit primitive would violate the HumanGL drawing contract.
6. **Animation is coupled to rendering.** `renderFrame()` advances auto-rotation
   and material blending while also clearing the framebuffer and uploading one
   model matrix. Calling it once per body part would clear previous parts and
   advance animation repeatedly. Also, `1 / 60` is integer zero in the current
   rotation condition; it is not a meaningful frame-time gate.
7. **GPU ownership needs care when expanding.** `Render` copies a `ShaderProgram`
   handle and deletes it in `cleanUp()`. Do not create ten renderers that each
   delete the same program. Texture objects also currently lack a texture-delete
   path. Use one shared renderer/mesh for the ten body draws and give each GPU
   resource a single owner.

Initial framebuffer pixel dimensions are not explicitly queried in the viewer.
The resize callback updates the viewport/aspect, but rendering should also be
initialized from actual framebuffer pixels and skip zero-size frames. These
changes belong with the camera/frame preparation step.

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
`MatrixTransform`, `Camera`, and the default model translation. Example assets
and the original scop project remain unchanged.

## Build the project in this order

1. **Prove the inherited viewer works.** Build and run the included cube/BMP
   command in the root README. Locate context creation, the shader program,
   `MatrixTransform`, the uniform upload, and the single indexed draw.
2. **Make the maths and camera trustworthy.** Retain row-major storage and degree
   rotation APIs. Projection and axis scaling are implemented and tested; correct
   the remaining Y convention and extend the point checks. Move the camera to
   view the origin from positive Z; set the initial model/global scale to 1 for
   the future unit body.
3. **Create one unit cube and one drawing method.** Add a small cube `Mesh` builder
   with positions ±0.5 and 36 indices, retaining Render's existing vertex layout
   initially. Add a flat body-color shader path and remove the texture requirement
   from the HumanGL body entry point. Upload the cube once.
4. **Separate frame work from part work.** Make `beginFrame(camera)` clear once
   and upload the view/projection. Make `drawUnitCube(modelMatrix, color)` upload
   one model/color, bind the shared VAO, and issue one `glDrawElements`. It must
   not advance time, clear the frame, or resize geometry.
5. **Add the stack and a two-part arm.** Save matrices by value. Multiply
   `parent * attachment * jointRotation`, temporarily apply center-offset and
   geometry scale for each draw, then restore the unscaled joint frame. Prove
   that growing an upper arm repositions its elbow without stretching its forearm.
6. **Build all ten parts.** Add dimensions and pose data plus a `Humanoid` traversal
   with torso/head, two arm chains, and two leg chains. Calculate attachments from
   dimensions each frame. Ten body parts must reach the cube drawing method once
   each; a parent joint must affect only its descendants.
7. **Add idle, walk, and jump above rendering.** Evaluate an animation controller
   once per frame using elapsed seconds. Input selects its state. It produces
   joint angles and root movement; the same hierarchy then draws every pose.
8. **Complete the defense checklist.** Check resizing, inheritance, stack balance,
   drawing calls, and both target platforms before adding bonuses.

Suggested additions are `MatrixStack` beside `MatrixTransform`, `Humanoid` beside
the scene classes, and an animation controller. The rest should extend the
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
- No skeleton or animation system has been implemented in this adoption step.
