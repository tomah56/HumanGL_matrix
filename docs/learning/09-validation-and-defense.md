# 9. Validate the project and prepare your defense

[Previous: animation](08-animation.md) · [Guide index](README.md) · [Research index](10-research.md)

## What you are learning

Validation connects a requirement to observable evidence. Test maths numerically,
inspect the actual drawing path, and demonstrate the articulated behavior. Passing
one kind of check does not prove the others.

The following is a checklist for the application you will build. The repository
currently contains documentation, so the program checks below are **not yet run**.

## Targeted numerical checks

As the relevant functions become available, maintain a small CPU-only test target
such as `make test`. It should run the meaningful cases from the guide:

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

Record this table when running the program. Never infer the Linux result from a
successful Mac build, or infer rendering success from compilation alone.

| Platform | Build | Actual context/renderer | Graphical checklist | Evidence |
| --- | --- | --- | --- | --- |
| macOS | Not run | Not recorded | Not run | Fill in machine, command, and observations. |
| Linux | Not run | Not recorded | Not run | Fill in machine, command, and observations. |

The intended application commands, after you implement their targets, are:

```sh
make
make test
./humangl
```

For release/defense readiness, also perform one clean rebuild (`make fclean`
followed by `make`) and verify the documented shader paths from the documented
working directory. Confirm a second unchanged `make` avoids needless recompiling.
These targets and executable do not exist in the current documentation-only repo.

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
