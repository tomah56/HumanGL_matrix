# HumanGL_matrix

An educational project about hierarchical modeling, custom matrix transformations,
and a matrix stack, used to animate an articulated person with modern OpenGL.

## Start learning

Read the [learning and research guide](docs/learning/README.md). It explains the
assignment and provides a staged route from rendering a triangle to animating the
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
| `src/render/` | VAO/VBO/EBO setup and the future shared unit-cube drawing function. |
| `src/matrixMath/`, `src/scene/` | Custom maths, camera, and root transformations to extend. |
| `src/modelLoader/`, `src/texture/` | Existing viewer path; useful for reference, optional for the body. |

## Current status

The independent C triangle starter has been removed. The repository contains the
scop-based viewer and learning material. Small import fixes cover portable GLFW
discovery, strict-compiler compatibility, timer initialization, and uploading
initialized matrices to the active program.

The matrix stack, unit cube, articulated body, and idle/walk/jump animation are
the next implementation stages. Known inherited maths issues and current
validation limits are recorded in the
[scop adaptation guide](docs/learning/11-scop-base.md). Future interfaces and
`make test` in the learning chapters describe work still to implement.
