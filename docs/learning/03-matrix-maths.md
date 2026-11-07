# 3. Implement your own matrix maths

[Previous: setup](02-rendering-setup.md) · [Guide index](README.md) · [Next: cube](04-unit-cube.md)

## What you are learning

A matrix can describe a coordinate frame: its axes and origin relative to another
frame. Multiplying frames lets you convert a child's coordinates into its
parent's coordinates, then into world coordinates.

Use a homogeneous point `(x, y, z, 1)` and a direction `(x, y, z, 0)`.
The fourth component lets a 4×4 matrix represent translation. Translation moves
a point; it leaves a direction unchanged.

## Define storage and multiplication

Start by inspecting scop's `src/matrixMath/MatrixTransform.*`. It already provides
array-based operations. The [adaptation review](11-scop-base.md) lists the concrete
corrections needed before relying on them for joints. Keep that code as the maths
base; the following are suggested value-type helpers to build around it as the
exercises progress, rather than a second independent maths library:

```cpp
typedef struct { float x, y, z; } Vec3;
typedef struct { float x, y, z, w; } Vec4;
typedef struct { float m[16]; } Mat4;

Mat4 mat4_identity(void);
Mat4 mat4_mul(Mat4 a, Mat4 b);
Vec4 mat4_apply(Mat4 m, Vec4 v);
Mat4 mat4_translation(float x, float y, float z);
Mat4 mat4_scale(float x, float y, float z);
Mat4 mat4_rotation_x(float degrees);
Mat4 mat4_rotation_y(float degrees);
Mat4 mat4_rotation_z(float degrees);
```

For row `r` and column `c`, store the value at `m[r * 4 + c]`. Storage layout
and the decision to use column vectors are distinct conventions; both are fixed
here. The multiplication rule in mathematical row/column notation is:

```text
C[r,c] = sum over k=0..3 of A[r,k] * B[k,c]
(M * v)[r] = sum over c=0..3 of M[r,c] * v[c]
```

Write the result into a separate matrix before returning it. Do not overwrite
values of an input that later terms still need. Passing and returning `Mat4` by
value is a simple starting interface.

## Build the elementary transforms

The following blocks show matrices as mathematical rows, not as memory arrays.
`c = cos(angle)` and `s = sin(angle)`. Positive rotations follow the right-hand
rule.

```text
I = [1 0 0 0]     T = [1 0 0 tx]     S = [sx 0  0  0]
    [0 1 0 0]         [0 1 0 ty]         [0  sy 0  0]
    [0 0 1 0]         [0 0 1 tz]         [0  0  sz 0]
    [0 0 0 1]         [0 0 0 1 ]         [0  0  0  1]

Rx = [1 0  0 0]    Ry = [ c 0 s 0]    Rz = [c -s 0 0]
     [0 c -s 0]         [ 0 1 0 0]         [s  c 0 0]
     [0 s  c 0]         [-s 0 c 0]         [0  0 1 0]
     [0 0  0 1]         [ 0 0 0 1]         [0  0 0 1]
```

Translation therefore occupies array elements 3, 7, and 11. Angles supplied to
`sinf` and `cosf` are radians: `radians = degrees * pi / 180`. Scop's public
rotation methods accept degrees and perform that conversion internally. Preserve
that API for the pose angles; use a project pi constant or C++20 `<numbers>` for
scalar calculations rather than relying on a nonstandard `M_PI` macro.

## Understand order and pivots

For column vectors, `T * R * S * p` scales first, rotates second, and translates
last. Matrix multiplication is generally not commutative.

Take `p = (1,0,0,1)`, `T = translate(2,0,0)`, and `R = rotate_z(90°)`:

- `T * R * p = (2,1,0,1)`.
- `R * T * p = (0,3,0,1)`.

To rotate geometry around a point `q` already expressed in the same frame, use
`T(q) * R * T(-q)`. A convenient body representation instead places the joint at
the local origin, rotates there, then offsets the cube away from that joint.

## Numerical exercises and expected results

Write a small CPU-only test executable as you implement the maths. Compare floats
using a tolerance such as `1e-5` for these small examples, rather than exact
equality. It should not need GLFW or OpenGL.

| Exercise | Expected result |
| --- | --- |
| Apply identity to `(1,2,3,1)` | `(1,2,3,1)` |
| Apply `T(2,3,4)` to `(1,2,3,1)` | `(3,5,7,1)` |
| Apply the same T to direction `(1,2,3,0)` | `(1,2,3,0)` |
| Apply `S(2,3,4)` to `(1,2,3,1)` | `(2,6,12,1)` |
| Rotate `(0,1,0,1)` by +90° around X | `(0,0,1,1)` |
| Rotate `(0,0,1,1)` by +90° around Y | `(1,0,0,1)` |
| Rotate `(1,0,0,1)` by +90° around Z | `(0,1,0,1)` |
| Rotate by an angle, then by its negative | Original point, within tolerance |
| Rotate `(2,0,0,1)` around pivot `(1,0,0)` by Z +90° | `(1,1,0,1)` |
| Multiply any test matrix by identity on either side | Original matrix |

Test the two order examples above too. A multiplication test containing only
identity matrices would not detect reversed operands.

## Research task and checkpoint

Read the matrix section of the [OpenGL uniform documentation](https://github.com/KhronosGroup/OpenGL-Refpages/blob/main/gl4/glUniform.xml).
Connect `transpose = GL_TRUE` to scop's row-major array representation. The
shader will receive your values; OpenGL does not construct your model matrix.

You are ready when you can calculate the order examples by hand, explain the
fourth component, and pass the numerical checks with your own functions.

## Common mistakes and defense questions

Mixing row-major array indices with column-major tutorial code, passing degrees to sine,
and reversing multiplication operands can all produce plausible but wrong images.
Check numerical points before changing rotation signs at random.

Why do matrices need 16 values? Why does translation not change a direction?
What does `parent * local` mean? How does a joint pivot differ from a cube center?
