# HumanGL_matrix

An educational project about hierarchical modeling, custom matrix transformations,
and a matrix stack, used to animate an articulated person with modern OpenGL.

## Start learning

Read the [learning and research guide](docs/learning/README.md). It explains the
assignment and provides a staged route from the scop viewer to animating the
complete body, with exercises, pseudocode, research links, and evaluation checks.

The implementation base is now your **scop project: C++20, GLFW, GLAD 2, and
OpenGL 4.1 core**. Its existing window, shaders, input, indexed rendering, and
custom matrix code are copied here. The original `../scop` project is unchanged.

Read [how to build HumanGL on scop](docs/learning/11-scop-base.md) for the source
review, issues to fix, and the sequence for adding the skeleton.

## Build the scop-based version

Install a C++20 compiler, Make, GLFW development files, and `pkg-config`. On
macOS with Homebrew, use `brew install glfw pkgconf`; on Ubuntu/Debian, use
`sudo apt install build-essential pkg-config libglfw3-dev`.

From this repository root:

```sh
make
./humangl --limbs
```

This shows a green torso with a tan head on top, two arms touching its sides,
and two legs attached underneath: ten blocks total. Every limb has an orange upper segment and a blue
lower segment. All four limbs share one pose and dimension set, bend forward
along local +Z, and resize together. Shoulders sit at the torso's upper sides;
hips sit along its bottom. The head's base touches the torso's top. Attachment
positions follow the torso dimensions.
Each segment is one scaled unit cube; elbows and knees follow the upper length.
The previous `--arms` and `--arm` commands are aliases for the same scene.

| Limb control | Effect |
| --- | --- |
| Q / E | Move shoulders and hips backward / forward together. |
| A / D | Decrease / increase elbow and knee bends together. |
| 1 / 2 | Shorten / lengthen upper arms and thighs together. |
| 3 / 4 | Shorten / lengthen forearms and lower legs together. |
| 0 | Restore all four limbs' default joint angles and dimensions. |
| Left / Right arrow | Turn the whole body left / right; hold to repeat. Shift turns faster. |
| Mouse drag | Rotate the whole group. |
| Escape | Close the window. |

Joint keys support holding/repeat; length controls stop at 0.2 world units.
Widths, depths, lengths, and initial pose can also be edited in
[Arm.hpp](src/scene/Arm.hpp). All four limbs use identical rotations about local X;
positive control angles bend forward along local +Z. The initial view is slightly
turned so their depth is visible. Torso width, height (`length`), and depth are
configured in [Humanoid.hpp](src/scene/Humanoid.hpp). Its model builder positions
the arms using torso width plus upper-arm width, and the legs using torso width.
Head dimensions are configured there too. Torso geometry scaling does not
stretch the head or attached limbs.

The earlier shared-cube demo remains available with `./humangl --cubes`.
This shows three colored cubes using one shared unit-cube mesh and a matrix
stack. The center cube and two sibling branches have independent local scales;
the left branch also has a local rotation. Drag with the mouse to rotate the
group; Cmd/Ctrl+R enables or disables automatic rotation. No OBJ/BMP is needed.

Run `make test` for projection/scaling, matrix-stack, cube-geometry, and limb checks.
They exercise the CPU code without opening a window or linking OpenGL/GLFW;
the project's development headers are still needed to compile the scene classes.
`make test-render` uses a hidden OpenGL window to check pixels, shared buffers,
one draw per cube, ten-part body rendering, and stack preservation. It needs a display
session. On a Linux machine with Xvfb/Mesa installed, run
`LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a make test-render`.

The original viewer path is also available:

```sh
./humangl models/cube.obj textureSources/wood.bmp
```

This runs scop's existing OBJ viewer under the HumanGL executable name. Escape
closes the window; mouse dragging rotates the model. Arrow keys and W/S translate
it, with Shift increasing the step. Cmd/Ctrl+R toggles automatic rotation;
Cmd/Ctrl+T toggles texture blending; Cmd/Ctrl+F toggles face/vertex colors. Scroll
changes model Z, despite the original viewer describing it as scaling.

`make clean`, `make fclean`, and `make re` retain scop's build workflow. The
generated loader is `glad.cpp` with headers in `include/`; it does not need
regeneration. All assets needed by the example command are included.

## What is reused

| Existing scop component | HumanGL role |
| --- | --- |
| `src/window/Window.*`, `src/inputHandler/` | Window, OpenGL 4.1 context, and input callbacks. |
| `src/shaders/`, `src/shaderSources/` | Shader compilation/linking and model/view/projection uniforms. |
| `src/render/` | Shared unit cube, VAO/VBO/EBO setup, frame preparation, and part drawing. |
| `src/matrixMath/`, `src/scene/` | Custom maths, matrix stack, camera, and root transformations. |
| `src/modelLoader/`, `src/texture/` | Existing viewer path; useful for reference, optional for the body. |

## Current status

The independent C triangle starter has been removed. The repository contains the
scop-based viewer and learning material. Small import fixes cover portable GLFW
discovery, strict-compiler compatibility, timer initialization, and uploading
initialized matrices to the active program.

Projection and scaling are corrected: perspective replaces all matrix entries,
and `MatrixTransform::scale(matrix, x, y, z)` composes local-axis scaling on
rotated matrices. The original uniform-scale overload remains available. The
default model starts at Z=−2, in front of the camera, and zero-size framebuffers
preserve the last valid projection aspect.

`MatrixStack` and `Render::drawUnitCube(stack, color)` are implemented. The
renderer uploads its cube once, draws it with one 36-index call per invocation,
and leaves the stack unchanged. `beginFrame(camera)` clears/uploads the camera
once per frame; scene updates happen separately. The renderer owns its mesh
buffers and fallback white texture, and borrows the shader program from its caller.

The articulated chain is implemented in [Arm.cpp](src/scene/Arm.cpp).
`Arm::modelMatrices(parent)` uses the stack to build two models, preserving the
attachment frame and keeping drawing scale out of joint inheritance.
`pairedModelMatrices(parent)` reuses that chain at left/right attachments with
identical angles, producing four model matrices from the same control state.
`limbModelMatrices(parent)` reuses that pair below it as legs, producing eight
model matrices with identical joint motion and shared sizing for the earlier
separated-pair stage. The current scene uses [Humanoid.cpp](src/scene/Humanoid.cpp)
to attach both pairs and the head to the torso and return all ten model matrices.

Idle/walk/jump animation is the next implementation
stages. Known inherited maths issues and current
validation limits are recorded in the
[scop adaptation guide](docs/learning/11-scop-base.md). Later learning chapters
describe tests for body, stack, and animation features still to implement.
