# 4. Draw one reusable unit cube

[Previous: maths](03-matrix-maths.md) · [Guide index](README.md) · [Next: stack](05-matrix-stack.md)

## What you are learning

Geometry describes a shape in its own coordinates. A model matrix places it in
the world. A view matrix expresses the world relative to the camera. A projection
matrix maps that view into clip space. The vertex shader calculates:

```text
clip_position = projection * view * model * vec4(local_position, 1)
```

OpenGL then clips primitives, performs the perspective divide, and maps the
result to the viewport. Keep these spaces separate when diagnosing an image.

## Build the shape once

Use eight corner positions at all combinations of ±0.5. Define six faces, each
with two triangles: 12 triangles and 36 indices. Use consistent outward winding.
For the first version, use an unlit uniform color; lighting and normal matrices
can wait until the required behavior works.

Upload the positions to a VBO and the indices to an element buffer. Bind a VAO
while setting the position attribute and element-buffer binding. Reuse these
GPU objects for every body part. If you later need per-face normals or colors,
duplicating corner vertices is fine; the geometric extent stays 1×1×1.

Read the VAO and index-buffer sections of
[Khronos's vertex specification guide](https://wikis.khronos.org/opengl/Vertex_Specification).
Use the binding-based APIs available in 4.1, rather than newer direct-state-access
APIs you may see elsewhere on that page.

## Establish a camera with your maths

Initially put the camera at `(0,0,6)`, looking along −Z toward the origin. Its view
matrix is `T(0,0,-6)`: moving the camera backward expresses the world farther in
front of the camera. Use this simple view before implementing an orbit camera.

For perspective, let `f = 1 / tan(vertical_fov / 2)`, `a = width / height`,
`n > 0` be near distance, and `F > n` be far distance. In row notation:

```text
P = [f/a  0       0              0       ]
    [ 0   f       0              0       ]
    [ 0   0  (F+n)/(n-F)  (2*F*n)/(n-F) ]
    [ 0   0      -1              0       ]
```

Try a 60° vertical field of view, near 0.1, and far 100. Recompute projection
when the nonzero framebuffer dimensions change. A point at view-space `z=-n`
maps to normalized depth −1; at `z=-F` it maps to +1 after dividing by w.
Use those two checks to verify your perspective function.

For a general camera later, implement your own look-at view construction. With
eye `e`, target `t`, and approximate up `u0`, calculate `f = normalize(t-e)`,
`s = normalize(cross(f,u0))`, and `u = cross(s,f)`. Its rows are:

```text
V = [ s.x  s.y  s.z  -dot(s,e)]
    [ u.x  u.y  u.z  -dot(u,e)]
    [-f.x -f.y -f.z   dot(f,e)]
    [  0    0    0       1    ]
```

Choose distinct eye/target positions and a nonparallel up direction; otherwise
normalization cannot define the camera axes. The `(0,0,6)` case above should
produce the same view matrix by both methods.

## The drawing contract

Use this conceptual interface:

```c
void draw_unit_cube(Renderer *renderer, const MatrixStack *stack, Vec3 color);
```

`Renderer` owns the shared cube, shader, and current projection/view matrices.
`MatrixStack` is introduced in the next chapter; its top contains the model
matrix. This function reads the top without changing it.

```text
draw_unit_cube(renderer, stack, color):
    model = copy of stack top
    bind the cube shader program
    upload projection, view, model, and color
    bind the shared cube VAO
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, offset 0)
```

For the standalone cube exercise, a one-entry stack containing your model matrix
is enough. Until that type exists, pass the model matrix directly as a temporary
learning step. The final body traversal uses the stack interface above.

The function's vertices always describe a unit cube at the local origin. It does
not accept a body-part name, rewrite vertex positions, choose a joint, or change
the stack. The caller supplies the transformed coordinate system. Read
[the indexed drawing reference](https://github.com/KhronosGroup/OpenGL-Refpages/blob/main/gl4/glDrawElements.xml)
to distinguish index count, index type, and byte offset.

## Exercise and checkpoint

Draw one cube with `M = T(2,0,0) * Rz(90°) * S(1,2,1)`. Its center should be
`(2,0,0)` and its long axis should run along world X. Verify the transformed
local point `(0,0.5,0,1)` is `(1,0,0,1)` numerically.

Enable `GL_DEPTH_TEST`; clear both color and depth each frame. View the cube at
an angle to see its volume, then return to your simple camera for debugging.

## Research task

Use the vertex and indexed-drawing references above to label your VAO setup:
which buffer supplies positions, which supplies indices, and how the draw count
relates to triangle count. Trace one cube vertex through model, view, projection,
and the perspective divide; record its coordinates in each space.

## Common mistakes and defense questions

A positive view-space Z position lies behind this camera. A zero near plane
breaks perspective. A stale depth buffer can hide new frames. If uniforms appear
ineffective, check the active program and uniform locations before changing maths.

Why is a cube sized 1×1×1 when its coordinates use 0.5? How do 36 indices fit into
one draw call? What part of `projection * view * model` changes when a knee bends?
