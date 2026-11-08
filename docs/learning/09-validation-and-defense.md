# 9. Validate the project and prepare your defense

[Previous: animation](08-animation.md) · [Guide index](README.md) · [Research index](10-research.md)

## What you are learning

Validation connects a requirement to observable evidence. Test maths numerically,
inspect the actual drawing path, and demonstrate the articulated behavior. Passing
one kind of check does not prove the others.

The following is a checklist for the complete HumanGL application. The repository
now contains the adopted scop viewer. Its build/render evidence is separate from
the still-pending body, body-resizing, and animation checks below. See the
[scop review](11-scop-base.md) for inherited issues and the implementation route.

## Targeted numerical checks

`make test` now runs projection/scaling regression checks, including near/far
planes, stale matrix entries, invalid inputs, rotated/nonuniform scale,
translation preservation, resize aspects, and default cube visibility. These are
CPU checks without an OpenGL context. Extend the target as the relevant functions
become available to cover the rest of the guide:

It also includes 100 matrix-stack assertions for copying/restoring frames,
composition, sibling independence, bounds, reset, and null inputs. A separate
CPU executable checks the procedural cube's extent, closed surface, winding,
and unit face areas. `testArm` now runs 549 arm/leg hierarchy assertions for
pivot positions, parent inheritance, isolated elbow rotation, shared resizing,
matching forward orientations, shoulder spacing, unchanged child dimensions, and
invalid inputs. These include the copied lower pair, shared movement, resizing,
row spacing, and inherited root rotation. The complete
body's remaining joint relationships are still to implement.

- Matrix identity, point/direction translation, nonuniform scaling, all three
  rotation axes, reversed multiplication order, and rotation around a pivot.
- Projection of the near/far planes and agreement between the simple camera and
  general look-at construction.
- Stack copy/restore, independent siblings, overflow/underflow, and unchanged
  state after a rejected stack operation.
- Shoulder/elbow/wrist positions in rest and rotated poses; equality between a
  segment endpoint and the next joint origin before/after resizing.
- Equal walk time simulated with different step counts, jump takeoff/apex/landing,
  a step crossing landing, pause, reset, and input-state transitions.

Use the expected values in the chapters. Avoid tests that only repeat an
implementation formula on both sides of the comparison. For a numerical failure,
print the operation, inputs, expected output, actual output, and tolerance before
looking at a graphical symptom.

## Mandatory acceptance checklist

- [ ] A documented clean checkout builds with the repository's Makefile or equivalent.
- [ ] Actual context information confirms OpenGL 4.1 or newer for this chosen configuration.
- [ ] All ten body parts are visible, including separately modeled upper/lower segments.
- [ ] The torso rotates and every descendant follows it.
- [ ] Rotating one shoulder moves its upper arm and forearm without moving other branches.
- [ ] Rotating one elbow moves only its forearm; rotating a knee moves only its lower leg.
- [ ] Upper-arm and thigh length changes reposition their descendant joints automatically.
- [ ] Torso width/height changes update the corresponding attachments automatically.
- [ ] The child segment keeps its own dimensions when its parent is resized.
- [ ] Idle is stationary; walking alternates limbs and travels; jumping rises and lands.
- [ ] Movement speed is consistent across different frame rates.
- [ ] The shared drawing function uses a fixed unit shape centered on the origin.
- [ ] Each body part calls that function exactly once; the body makes ten indexed draws per render.
- [ ] Stack depth is balanced at the end of every body render.
- [ ] Window resize/minimize is handled, and shutdown releases resources with a current context.
- [ ] Submitted code, headers, shaders, loader files, license information, and instructions are in Git.

Temporarily count calls inside the shared primitive function when checking the
body's ten draws. Inspect its body to verify it issues a single indexed draw for
36 indices. Count other scene/debug rendering separately, or disable it during
this check. Draw counts alone do not prove geometry size or articulation.

## Platform evidence

Record evidence for the scop-based code, rather than carrying over results from
the removed triangle starter. A colored cube only proves the rendering path;
it does not prove that all matrix operations or joint relationships are correct.

| Platform | Build | Actual context/renderer | Graphical checklist | Evidence |
| --- | --- | --- | --- | --- |
| macOS, Apple Silicon | Passed, Apple Clang 21, warnings as errors | OpenGL 4.1 Metal | Hidden cube render probe passed; full body and manual interaction pending | Actual imported window/shaders/renderer/loaders; 36 indices, colored center pixel, no OpenGL error. |
| Ubuntu 24.04, ARM64 container | Passed, GCC, warnings as errors | OpenGL 4.5 core, Mesa 25.2.8 llvmpipe | Hidden cube render probe passed; full body and desktop interaction pending | Actual imported modules under Xvfb; 36 indices, colored center pixel, no OpenGL error. |

The Linux build first exposed missing integer-type declarations in the BMP
header; explicit standard types fixed it, and the subsequent build/render check
passed. A clean parallel rebuild passed on macOS during adoption. Projection and
scaling were subsequently corrected and have a regression suite; the remaining
maths/hierarchy work is listed in the adaptation guide.

On 7 November 2026, `make -j4 test all` passed on macOS and the equivalent
`make -j2 all test` passed in Ubuntu: 97 assertions on each platform. Updated
hidden-window probes also rendered the cube with the application's default model
transform and the corrected projection on both platforms, without OpenGL errors.
Keyboard/mouse interaction and the full body checks remain outside these checks.

The current viewer commands, from the repository root, are:

```sh
make
make test
./humangl --limbs
./humangl --cubes
./humangl models/cube.obj textureSources/wood.bmp
```

Extend the CPU-only `make test` target when adding joints and other maths.
The removed triangle starter's `make check`/`--check` interface is not part
of scop; the review used a separate temporary probe linked to the actual modules.

The shared-cube stage adds `make test-render`, which runs a permanent hidden-window
integration test. It checks two colored cubes in the same frame, exactly one draw
per cube, a shared VAO, no per-part geometry upload or clear, stack preservation,
the legacy mesh color mode, and safe renderer/program cleanup. It requires a
desktop session; under Linux/Mesa/Xvfb use
`LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a make test-render`.

On 7 November 2026, the shared-cube/stack stage passed `make test` and
`make test-render` on Apple M1 Pro/OpenGL 4.1 and Ubuntu/Mesa llvmpipe/OpenGL 4.5.
The final renderer check includes the white sampler fallback and exits without
OpenGL errors. The interactive `--cubes` mode is built; keyboard/mouse behavior
has not been automated or manually exercised by these checks.

The aligned-arm stage runs 420 arm assertions plus the existing suites, including
positive-forward flexion, matching rotations, and constant separation. The render test checks
orange upper arms and blue forearms on both sides, a gap between the arms,
exactly four 36-index draws, no new geometry upload, and a preserved parent
frame. Both arms use the same pose/dimensions and existing key controls;
physical keyboard/mouse interaction remains a manual check.

On 8 November 2026, the forward-facing version passed all CPU suites and the
hidden-render checks on macOS/OpenGL 4.1 and Ubuntu/Mesa/OpenGL 4.5. Both chains
use identical forward-bending rotations; the arm suite now contains 420 assertions.

The copied-leg stage subsequently passed the macOS build, all CPU suites
(including 549 limb hierarchy assertions), and the hidden OpenGL 4.1 render
check. It verifies eight colored limb blocks, the vertical gap, exactly eight
cube draws, no geometry reupload, and the unchanged parent frame. All four limbs
share one pose and dimension set. Linux and physical input checks for this
stage remain pending.

For release/defense readiness, also perform one clean rebuild (`make fclean`
followed by `make`) and verify the documented shader paths from the documented
working directory. Confirm a second unchanged `make` avoids needless recompiling.
The imported Makefile supplies these cleanup/rebuild targets and the viewer
executable. No complete HumanGL or animation validation is implied.

## Exercise: a short defense walkthrough

1. Build and start the program, show context information, and point out all ten parts.
2. Show idle, walking, jumping, landing, and state changes using the documented controls.
3. Isolate torso, shoulder, elbow, hip, and knee rotations using a debug pose or
   temporarily edited pose values. Explain which descendants should move.
4. Change the configuration for one upper arm, both thighs, and torso dimensions.
   Rebuild if needed and show all connections remain correct.
5. Show the unit-cube vertices, the one drawing function, and the traversal calls.
6. Trace the forearm's model matrix from the torso through the shoulder and elbow.
7. Explain the stack contents before/after drawing a segment and returning to a sibling.

Practise explaining `joint * T(center) * S(dimensions)` versus
`joint * T(next_joint) * R(next_angle)` with a drawing. Be able to reproduce the
small arm calculation from Stage 6 without running the program.

## Bonus roadmap

Only begin this after every mandatory acceptance item works without malfunction:

| Extension | What you would learn |
| --- | --- |
| Hands, feet, neck, or fingers | Extend the tree with more local frames and unit-shape calls. |
| Dance or fighting patterns | Author new joint-angle curves and state transitions. |
| Runtime dimensions and colors | Edit the existing configuration/pose inputs without changing hierarchy logic. |
| Pose blending | Interpolate between poses while keeping joint limits and conventions clear. |
| Lighting | Add normals and correctly transform them under nonuniform scaling. |
| Foot planting | Explore inverse kinematics after the forward hierarchy is proven. |

Re-run the affected mandatory checks after a bonus changes traversal or animation.
Keep the working mandatory version recoverable in Git before expanding it.

## Research task, mistakes, and checkpoint

For every checked item, identify its evidence: a numerical assertion, a source
inspection, a clean build, or a visible behavior. Revisit the source links in the
[research index](10-research.md) for claims you cannot explain confidently.

Do not label a platform tested without running there. Do not let a smooth
animation conceal an incorrect joint pivot. You are ready for defense when you
can explain both the observed result and the matrix relationship that produces it.
