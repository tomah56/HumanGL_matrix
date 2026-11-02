# 2. Establish the rendering setup

[Previous: assignment](01-assignment.md) · [Guide index](README.md) · [Next: maths](03-matrix-maths.md)

## What you are learning

A window displays pixels. An OpenGL context holds rendering state and connects
your OpenGL calls to an implementation. GLFW manages the window, context, input,
and timing. GLAD loads OpenGL function addresses. Your shaders tell the GPU how
to transform vertices and color fragments.

First establish a minimal render loop. Keep this stage separate from body maths
so initialization failures are easy to isolate.

## Prerequisites on each platform

Install a C11 compiler, Make, a GLFW development package, and a `pkg-config`
implementation. On macOS, use the Xcode command-line tools and an installed GLFW
package. On Linux, use your distribution's compiler/build tools and GLFW
development package, with a working graphics driver and desktop display session.
Package names vary; the discovery name used by this guide is `glfw3`.

These are prerequisite checks you can run before writing the application:

```sh
cc --version
make --version
pkg-config --modversion glfw3
pkg-config --cflags --libs glfw3
```

If GLFW is installed but cannot be discovered, find its `glfw3.pc` file and check
the package manager's suggested `PKG_CONFIG_PATH`. A missing package description
is a build-discovery issue; it does not prove that your graphics driver is wrong.

Use the [GLFW build guide](https://www.glfw.org/docs/latest/build_guide.html) to
understand discovery and linking. Prefer its `pkg-config` approach over paths
hard-coded to one Mac installation.

## Prepare a consistent GLAD loader

Use the [GLAD 2 generator](https://gen.glad.sh/): select C, OpenGL (`gl`) 4.1,
core profile, and no optional extensions. Generate normal source/header files.
Use GLFW's function resolver, so GLAD's internal loader option is unnecessary.
Keep the generated source, all generated headers, and license information in
the repository when implementing the application. Ordinary builds should not
need to contact the generator.

The naming in this guide is GLAD 2: `glad/gl.h`, `gl.c`, and
`gladLoadGL(glfwGetProcAddress)`. Older tutorials may use a different GLAD API.
Follow the [GLAD C documentation](https://github.com/Dav1dde/glad/wiki/C) that
matches the files you generated.

## Initialization order

Use the [GLFW getting-started guide](https://www.glfw.org/docs/latest/quick_guide.html)
for API mechanics. Adapt its example to the versions below and implement your own
maths later; do not copy its external maths helper.

```text
define GLFW_INCLUDE_NONE before including GLFW
include the GLAD and GLFW headers
install a GLFW error callback
initialize GLFW; stop with an error if it fails
request OpenGL major=4, minor=1, core profile, forward-compatible=true
create the window; stop with the callback error if it fails
make its context current
load OpenGL functions through GLAD; check success and 4.1 availability
print actual OpenGL version, shading-language version, renderer, and vendor
create shader program and triangle buffers/VAO; check errors
enable depth testing when moving to 3D geometry
loop: process input, update, clear, draw, swap buffers
delete GPU objects while the context is still current
destroy window and terminate GLFW
```

Requesting a context and printing its actual version are separate steps. Report
an unsupported version clearly instead of quietly running below the target.
macOS provides forward-compatible core contexts for modern OpenGL; see
[GLFW's platform notes](https://www.glfw.org/docs/latest/compat_guide.html).
OpenGL is deprecated there, but this assignment explicitly requires it.

## The first triangle

A VBO stores vertex data; a VAO describes how attributes read that data. Create
and bind a real VAO in core profile. Upload three positions and draw one triangle.
At this stage the vertex shader may put those positions directly in clip space;
the matrix exercise comes next. Use `#version 410 core` in both shaders.

Check shader compilation and program linking independently and print their info
logs on failure. A successfully created context does not prove that a shader
compiled. Start with a constant fragment color and a contrasting clear color.

Use the **framebuffer** dimensions for `glViewport` and for the aspect ratio.
They can differ from logical window dimensions, notably on Retina displays.
Skip rendering and projection updates when the framebuffer height or width is
zero, while continuing to process events. See the
[GLFW window guide](https://www.glfw.org/docs/latest/window_guide.html).

## Build exercise

When source files exist, write a Makefile with `all`, `clean`, `fclean`, and `re`
targets. Compile your C files and generated GLAD source with its include directory;
use `pkg-config` for GLFW flags and the platform maths link flags where needed
(`-lm` on Linux). With this resolver-based GLAD configuration, rendering calls go
through loaded pointers. If you instead add direct OpenGL platform calls, revisit
the platform-specific link requirements.

Use `-std=c11 -Wall -Wextra` for your code. Track header dependencies and avoid
rebuilding unchanged objects on every `make`. Keep shader files in the repository;
document running from the repository root if their paths are relative to it.

## Research task

Using the GLFW lifecycle and GLAD documentation above, annotate your own
initialization sequence with the owner of each resource and its matching cleanup
operation. Identify the first point at which each OpenGL call is valid. Explain
why a build dependency being installed does not guarantee a supported context.

## Checkpoint and common mistakes

You should see a triangle, close the window cleanly, and read an actual OpenGL
version of at least 4.1. Record OS, renderer, version, and build command on each
platform when you run it. Neither platform is validated by this guide alone.

If the screen is blank, check shader logs, VAO binding, viewport dimensions, and
the draw call before adding new features. Loading GLAD before making the context
current, mixing loader generations, or requesting a 3.3 tutorial context are
common setup mistakes.

## Explain it aloud

What is a context? Why load function pointers after creating it? What does a VAO
remember? Which files and dependencies would another evaluator need to build?
