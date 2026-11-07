# HumanGL: learning and research

You are building a person from ten rigid shapes. Every shape has its own local
coordinate system. A shoulder belongs to the torso; an elbow belongs to the upper
arm. Multiplying matrices connects these coordinate systems. A stack saves a
parent's coordinate system while you draw its children.

The result is a small skeletal animation system based on rigid parts. Understanding
how a parent's movement reaches its descendants is the main objective.

This guide assumes you can write C/C++ and have some graphics experience. It uses
explanations, exercises, and pseudocode rather than a complete application. Code
blocks labeled `text` are algorithms to translate into C++; declarations describe
suggested interfaces. Shell commands are labeled with their prerequisites.

Start with the [scop adaptation review](11-scop-base.md). The implementation now
reuses your C++20 scop code, including its custom row-major maths. Stage 2 walks
through that existing pipeline; later types and interfaces describe additions.

## Route through the project

Work in this order. Advance when you can demonstrate the checkpoint and explain
why it works; a rendered picture alone can hide incorrect maths.

| Stage | Read | Build checkpoint |
| --- | --- | --- |
| 1 | [Understand the assignment](01-assignment.md) | Explain the ten parts and every mandatory constraint. |
| 2 | [Rendering setup](02-rendering-setup.md) | Build and explain the copied scop OBJ viewer. |
| 3 | [Your matrix maths](03-matrix-maths.md) | Pass numerical transformation exercises without a window. |
| 4 | [One reusable cube](04-unit-cube.md) | Transform the same unit cube using your matrices. |
| 5 | [Your matrix stack](05-matrix-stack.md) | Draw independent branches with balanced push/pop operations. |
| 6 | [Body hierarchy and joints](06-body-hierarchy.md) | Build the full articulated person. |
| 7 | [Adjustable proportions](07-proportions.md) | Resize a part and preserve its joint connections. |
| 8 | [Idle, walk, and jump](08-animation.md) | Animate poses using elapsed time. |
| 9 | [Validation and defense](09-validation-and-defense.md) | Demonstrate and explain all mandatory behavior. |

Each chapter includes a research task, an exercise, expected results, common
mistakes, and questions for your defense. The [research index](10-research.md)
collects the primary documentation and explains how to use it.

## Conventions used throughout

Keep these conventions consistent in your C++ and GLSL:

| Topic | Choice |
| --- | --- |
| Language | C++20, retaining scop's classes and custom maths. |
| Platforms | macOS and Linux with a working desktop OpenGL 4.1 implementation. |
| Rendering | OpenGL 4.1 core; shaders use `#version 410 core`. |
| Window and input | GLFW; GLAD 2 loads OpenGL functions. |
| Maths ownership | Your code implements matrices, transforms, camera matrices, and stack. |
| Coordinates | Right-handed, +X right, +Y up; character faces +Z in its rest pose. |
| Camera | Initially on the +Z side, looking toward the origin along −Z. |
| Vectors and matrices | Column vectors; row-major `float[16]` storage, uploaded with `GL_TRUE`. |
| Composition | `A * B` applies B first; stack multiplication is `top = top * local`. |
| Angles | Degree-valued rotation APIs/poses, matching scop; radians for sine/cosine and animation phase. |
| Geometry | Shared procedural cube: coordinates −0.5 to +0.5. The separate viewer OBJ remains −1 to +1. |
| Body | Torso frame at its center; limbs extend along local −Y from their joints. |

The assignment permits OpenGL 4.0 or newer. This guide chooses **4.1** for one
shared target on macOS and Linux. A Linux system limited to 4.0 could meet the
assignment but would need a separate 4.0/GLSL 400 configuration. Do not silently
fall back to an older context. See [GLFW's macOS guidance](https://www.glfw.org/faq).

## Keep the architecture small

Use four responsibilities as the application develops:

- **Maths:** vectors, matrices, camera construction, and the matrix stack; no OpenGL calls.
- **Rendering:** shaders, the shared cube mesh, matrix/color uniforms, and drawing.
- **Body:** dimensions, attachment positions, joint traversal, and one cube call per part.
- **Animation/application:** input, state, elapsed time, poses, and root movement.

The data flow should be easy to trace:

```text
input + elapsed time -> animation state -> joint pose + root position
dimensions + pose -> stack traversal -> model matrix for each body part
projection + view + model -> vertex shader -> image
```

These are responsibilities, not a requirement to write a large framework. Start
with small functions and split files as the responsibilities become clear.

## Record what you learn

For each stage, keep a short note in the repository: what you predicted, what you
observed, the cause of one mistake, and how you proved the fix. Mark checkpoints
only after checking them. The validation chapter includes a platform evidence
table for the adopted scop base and the complete project's pending checks.

Your first task is [Stage 1: understand the assignment](01-assignment.md).
