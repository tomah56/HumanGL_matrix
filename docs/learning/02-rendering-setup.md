# 2. Understand and reuse scop's rendering setup

[Previous: assignment](01-assignment.md) · [Guide index](README.md) · [Next: maths](03-matrix-maths.md)

## What you are learning

A window displays pixels. An OpenGL context holds rendering state and connects
OpenGL calls to an implementation. Your scop project already creates that context,
loads OpenGL functions, compiles shaders, uploads a mesh, and handles input.
Understanding that existing path is the first exercise for HumanGL.

The implementation is C++20. The conceptual exercises can still use simple
structs and functions, but you should extend the existing C++ classes. Read the
[scop adaptation review](11-scop-base.md) for the exact reuse map and known gaps.

## Build and run the existing path

Install a C++20 compiler, Make, GLFW development files, and `pkg-config`.
On macOS use the Xcode command-line tools and `brew install glfw pkgconf`.
On Ubuntu/Debian use `sudo apt install build-essential pkg-config libglfw3-dev`.
A graphics driver/display session must support OpenGL 4.1 or newer.

These prerequisite checks do not open a window:

```sh
c++ --version
make --version
pkg-config --modversion glfw3
pkg-config --cflags --libs glfw3
```

The Makefile retains scop's object/dependency layout, warning flags, and debug
option (`make DEBUG=1 re`). GLFW is discovered using `pkg-config` on either OS;
the build does not assume a particular Homebrew prefix or replace your package
search path. Inspect `all`, `clean`, `fclean`, and the sequential `re` target.

From the repository root:

```sh
make
./humangl models/cube.obj textureSources/wood.bmp
```

That command retains the OBJ/BMP viewer. `./humangl --cubes` now runs a procedural
unit-cube demo with plain colors and a matrix stack, without asset arguments.
The included OBJ is 2×2×2; it is a viewer example rather than the unit primitive.

## Follow the code you already have

1. `main.cpp` loads the OBJ mesh using `RenderModelLoader`.
2. `Window` initializes GLFW and creates a 4.1 core, forward-compatible context.
3. `gladLoadGL(glfwGetProcAddress)` loads OpenGL functions after the context is current.
4. `ShaderProgram` compiles and links `src/shaderSources/default.vert` and `.frag`.
5. `Render` creates the VAO, vertex buffer, and index buffer for the mesh.
6. `Texture` loads the BMP. The active shader's sampler selects texture unit zero.
7. `Transformation` and `Camera` produce model/view/projection matrices; `Render`
   uploads them, then the indexed draw sends the mesh to the GPU.
8. GLFW swaps buffers and processes input; cleanup must happen before destroying
   the context.

Read the corresponding classes beside this list. `glad.cpp`, `include/glad/gl.h`,
and `include/KHR/khrplatform.h` are the copied loader files. Their header records
OpenGL 4.1 core, zero optional extensions, and GLAD 2.0.8. Keep these matching
files together; ordinary builds do not need a generator or Python.

## Understand what changes for a body

Scop currently uploads one model matrix for one OBJ. HumanGL needs a different
model matrix for every body part, while sharing the camera and cube mesh.
`beginFrame(camera)` now clears and uploads the camera once per frame.
`drawUnitCube(stack, color)` changes only the per-part drawing inputs and issues
one draw. Viewer animation/material state advances once in `main.cpp` before
matrix evaluation. The old combined `renderFrame()` has been replaced.

The vertex shader already calculates `projectionMatrix * viewMatrix *
modelMatrix * vec4(aPos, 1)`. CPU matrices are row-major and uploaded with
`GL_TRUE`. That is a coherent convention; Stage 3 explains its array indices.

`Window::getFramebufferSize()` now supplies actual pixel dimensions to the main
loop. It updates viewport/aspect from those dimensions and skips zero-size frames
while processing events. The resize callback remains available.

## Research task and exercise

Use the [GLFW getting-started guide](https://www.glfw.org/docs/latest/quick_guide.html)
for API mechanics and the [build guide](https://www.glfw.org/docs/latest/build_guide.html)
for linking. Annotate the eight-step path above with the owning class, resource
lifetime, and matching cleanup call. Mark which operations happen once, once per
frame, and eventually once per body part.

Find the three matrix uploads in `RenderDraw.cpp`. Predict how many model uploads
and draw calls the ten-part body will need, and how many times the framebuffer
should be cleared. Then explain why replacing the existing pipeline is unnecessary.

## Checkpoint, common mistakes, and defense

You are ready to continue when you can build the copied viewer, locate its matrix
upload and draw call, and explain the context/shader/mesh lifecycle. Current build
and rendering evidence is in the [validation chapter](09-validation-and-defense.md).
The [adaptation review](11-scop-base.md) explains inherited maths defects that
must be fixed before the body is evaluated.

Do not regenerate a second loader, mix row-major and column-major indices, or
mistake the existing cube asset for a unit cube. If the viewer fails, read the
specific window/shader/model error before changing the maths. A working viewer
is a reusable base, not proof that the future hierarchy is correct.
