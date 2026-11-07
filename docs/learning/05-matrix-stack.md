# 5. Implement your matrix stack

[Previous: cube](04-unit-cube.md) · [Guide index](README.md) · [Next: hierarchy](06-body-hierarchy.md)

## What you are learning

A stack is last-in, first-out storage. Here each entry is a complete model matrix.
Pushing makes a copy of the current frame. Local transformations change the top
copy. Popping restores the previous frame, including its origin and axes.

This solves a concrete problem: after drawing the left arm, the right arm must
start from the torso frame, not from the left wrist.

## Define a small interface

The implemented class is [MatrixStack](../../src/matrixMath/MatrixStack.hpp),
with its operations in [MatrixStack.cpp](../../src/matrixMath/MatrixStack.cpp).
Run `make test` for its CPU checks and `./humangl --cubes` for two independent
sibling branches drawn from one saved parent.

An array with capacity 32 is ample for this hierarchy. Initialize it with one
identity matrix. Here `count` means the number of occupied entries; top is always
`entries[count - 1]`.

```cpp
MatrixStack stack;                         // one identity matrix
stack.load(rootModelMatrix);               // copy a row-major float[16]
if (!stack.push()) throw std::runtime_error("Stack overflow");
stack.translate(0.0f, -upperLength, 0.0f);   // top = top * local translation
stack.rotateX(elbowDegrees);
const auto elbowFrame = stack.top();       // independent std::array<float, 16>
if (!stack.pop()) throw std::runtime_error("Stack underflow");
```

The class stores 32 `std::array<GLfloat, 16>` values. `size()` reports depth and
`reset()` restores one identity entry. `load()` and `multiply()` accept either a
matrix array value or a pointer to 16 floats. `translate()`, `rotateX/Y/Z()`, and
uniform/nonuniform `scale()` build local matrices and post-multiply the top.
They retain scop's existing degree-based rotation conventions, including the
pending Y-direction issue noted in the adaptation guide.

The first entry must remain present. On overflow or a pop of the last entry,
return false without changing the stack. In a body traversal, treat such a failure
as a programming error: report it and stop that render rather than proceeding
with an incorrect frame.

```text
push:
    if count == capacity: return false
    entries[count] = entries[count - 1]     // copy all 16 values
    count += 1
    return true

pop:
    if count <= 1: return false
    count -= 1
    return true

multiply(local):
    entries[count - 1] = mat4_mul(entries[count - 1], local)
```

`top()` returns a value copy. Storing pointers to one mutable matrix in every
entry would make all frames change together and defeat saving/restoring.

## Trace two branches

```text
initialize stack
load parent frame P

push
    multiply T(left attachment)
    multiply R(left joint)
    draw left branch
pop

push
    multiply T(right attachment)
    multiply R(right joint)
    draw right branch
pop

assert top == P and count == 1
```

The indentation expresses lifetime: transformations belong to the matching
push/pop interval. A recursive traversal can use the same pattern, but the first
body implementation can be explicit functions for the few known branches.

## Separate a joint frame from a shape frame

Keep the joint on the stack while drawing a segment. Temporarily add the cube's
center offset and dimensions; remove them before locating the next joint.

```text
// Stack top is the shoulder frame, already rotated by its joint angle.
push
    multiply T(0, -upper_length/2, 0)
    multiply S(upper_width, upper_length, upper_depth)
    render.drawUnitCube(stack, color)
pop
// Stack top is the shoulder again; its units have not been stretched.
multiply T(0, -upper_length, 0)
multiply R(elbow_angle)
// Stack top is now the elbow frame.
```

This is the central stack pattern of the project. Body dimensions affect where
a child is attached without leaking the parent's drawing scale into the child.

## Exercise and checkpoint

Add CPU-only checks alongside your matrix checks:

These cases, plus copied-value semantics, rotated local translations, scale
isolation, reset, and null inputs, are already covered by `make test`. Use them
as exercises to predict the results before reading the assertions.

1. Initialize and verify identity plus count 1.
2. Load `P = T(3,0,0)`, push, and multiply `T(0,2,0)`.
3. The top applied to the origin must produce `(3,2,0,1)`.
4. Pop; the restored top must produce `(3,0,0,1)`.
5. Repeat for a second branch with `T(0,-2,0)`; expect `(3,-2,0,1)`.
6. Fill the stack to capacity. Another push must fail without changing top/count.
7. Pop back to count 1. Another pop must fail without changing top/count.

In the graphical exercise, draw two cubes from the saved parent frame. Rotating
one branch must leave the other unchanged. Verify each frame begins and ends at
the same depth, including after hundreds of renders.

## Research task

Sketch stack contents beside the shoulder/elbow pseudocode. Label which entries
are joint frames and which are temporary geometry frames. Relate the frame values
to the matrix upload described in the [uniform reference](https://github.com/KhronosGroup/OpenGL-Refpages/blob/main/gl4/glUniform.xml).
Your stack lives in C++ memory; the shader receives a matrix value for each draw.

## Common mistakes and defense questions

An unbalanced pop can break the next branch. A missing pop after scaling can
stretch children. Multiplying `local * top` changes the chosen composition
convention. Rebuilding the stack from a known root each frame prevents accidental
accumulation, but does not replace balanced operations within that frame.

What exactly does push copy? Why must a pop preserve the base entry? At what line
does the elbow inherit the shoulder rotation? Why should view/projection matrices
remain outside the model stack?
