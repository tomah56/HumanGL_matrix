# Research index

[Guide index](README.md) · [Validation and defense](09-validation-and-defense.md)

These are primary sources for the chosen tools and API. Read a focused section
when its exercise needs it. Record the page/section, one fact you learned, and a
small experiment that checks your understanding. Reference documentation explains
an API contract; your numerical examples establish your own maths conventions.

## Window, build, and loader

| Source | Read for | Question to answer |
| --- | --- | --- |
| [GLFW introduction](https://www.glfw.org/docs/latest/) | Library responsibilities and documentation map. | What work remains your application's responsibility? |
| [GLFW getting started](https://www.glfw.org/docs/latest/quick_guide.html) | Context setup and the basic application lifecycle. | At which point is it valid to initialize the OpenGL loader? |
| [GLFW building applications](https://www.glfw.org/docs/latest/build_guide.html) | Include options, `pkg-config`, and linking. | Which build flags come from GLFW discovery? |
| [GLFW FAQ](https://www.glfw.org/faq) | macOS modern-context requirements and framebuffer size issues. | Why choose a 4.1 core context for this guide? |
| [GLFW platform compatibility](https://www.glfw.org/docs/latest/compat_guide.html) | Platform restrictions and macOS OpenGL status. | Which assumptions should you verify on each target machine? |
| [GLFW window guide](https://www.glfw.org/docs/latest/window_guide.html) | Framebuffer sizes, events, and context hints. | Why can logical window size differ from viewport pixel size? |
| [GLAD 2 generator](https://gen.glad.sh/) | Generate bindings for C, OpenGL 4.1 core. | Which generated files must accompany the source? |
| [GLAD C guide](https://github.com/Dav1dde/glad/wiki/C) | Loading through GLFW and integrating generated files. | Why must loader API names match the generator version? |

Adapt tutorials deliberately: select a 4.1 context and GLSL 410, implement your
own maths instead of copying helper libraries, and keep required generated files
in Git. The GLFW example's 3.3 context does not meet this assignment's minimum.

## Rendering and matrix transfer

| Source | Read for | Question to answer |
| --- | --- | --- |
| [Khronos vertex specification](https://wikis.khronos.org/opengl/Vertex_Specification) | VAOs, buffer-backed attributes, and index buffers. | Which vertex and index state is associated with a VAO? |
| [Khronos indexed drawing reference source](https://github.com/KhronosGroup/OpenGL-Refpages/blob/main/gl4/glDrawElements.xml) | Parameters and behavior of `glDrawElements`. | Is count measured in indices, bytes, or triangles? |
| [Khronos uniform reference source](https://github.com/KhronosGroup/OpenGL-Refpages/blob/main/gl4/glUniform.xml) | Matrix upload and transpose conventions. | Why does this guide use `GL_FALSE` for transpose? |
| [Khronos OpenGL reference index](https://registry.khronos.org/OpenGL-Refpages/gl4/html/index.php) | Look up individual API calls as you use them. | What is each call's minimum supported version? |
| [Khronos specification registry](https://registry.khronos.org/OpenGL/index_gl.php) | The OpenGL 4.1 core and GLSL 4.10 specifications. | Which behavior is required by the version you requested? |

The two reference-source links show the official reference text as XML on GitHub;
the content can be read in its code view. They are useful when the formatted
reference pages cannot be retrieved by a documentation tool.

The broader reference/wiki pages also discuss APIs newer than 4.1. For this
project, use the binding-based buffer/VAO setup and basic uniforms described in
the exercises. Check availability before copying a newer convenience function.

## Timing and input

Read [GLFW's input guide](https://www.glfw.org/docs/latest/input), especially key
events and time input. Explain the difference between a press event and repeat,
and between elapsed seconds and rendered frame count. Validate that distinction
using the different-step-count animation exercise in Stage 8.

## Your own research experiments

The maths and hierarchy chapters provide derivations to reproduce, rather than
requiring a matrix library. Keep these experiments beside your implementation:

| Topic | Experiment |
| --- | --- |
| Homogeneous coordinates | Apply the same translation to a point and a direction. |
| Noncommuting transforms | Compare `T * R * p` and `R * T * p` numerically. |
| Joint pivots | Rotate the two-segment arm and predict elbow/wrist positions. |
| Stack restoration | Change a child, pop, and compare the restored parent. |
| Dimension dependencies | Grow a parent and compare its endpoint with the child origin. |
| Projection | Check normalized depth at near and far planes. |
| Animation | Evaluate identical elapsed times using different update counts. |

A short research note can use this format:

```text
Question:
Prediction and reason:
Source and section, if relevant:
Small experiment:
Expected result:
Observed result:
What this proves, and what it does not prove:
```

External documentation can change. Keep the chosen versions, conventions, and
build requirements documented in the repository so the project remains
understandable during evaluation. These links were consulted while preparing
the guide; graphical platform validation remains a separate implementation task.
