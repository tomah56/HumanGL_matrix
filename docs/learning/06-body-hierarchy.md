# 6. Build the articulated person

[Previous: stack](05-matrix-stack.md) · [Guide index](README.md) · [Next: proportions](07-proportions.md)

## What you are learning

The first two-part chain is now implemented in [Arm.cpp](../../src/scene/Arm.cpp).
Run `./humangl --arm` for an upper arm and forearm, then use Q/E for the shoulder,
A/D for the elbow, 1/2 for upper-arm length, and 3/4 for forearm length.
`0` restores the default arm dimensions and pose. The demo uses local Z rotation
for a clear front view; the X-axis movement described below is a later extension
for the complete body's walking pose.

`Arm::modelMatrices(parent)` evaluates the hierarchy on a copy of the parent
stack and returns the two geometry model matrices. It rotates the shoulder,
temporarily adds center-offset/scale for the upper block, restores the joint,
translates by the upper length to reach the elbow, then rotates and shapes the
forearm. The caller sends each result to the same `drawUnitCube()` method once.
The remaining eight parts are still to implement.

A joint frame is a moving coordinate system. For a child joint:

```text
child_world = parent_joint_world * T(parent_relative_attachment) * R(child_angle)
```

For a cube representing a downward segment of length `L`:

```text
segment_model = joint_world * T(0, -L/2, 0) * S(width, L, depth)
```

The segment's upper end is at its joint origin; its lower end is at local
`(0,-L,0)`. This representation makes elbows and knees straightforward to locate.

## Choose rest dimensions and attachment rules

Use world units consistently; their physical unit is your choice. These values
are a starting configuration, not a requirement of the assignment:

| Shape | Width | Height/length | Depth |
| --- | --- | --- | --- |
| Torso | 1.2 | 1.8 | 0.6 |
| Head | 0.6 | 0.7 | 0.6 |
| Each upper arm | 0.3 | 1.0 | 0.3 |
| Each forearm | 0.25 | 0.9 | 0.25 |
| Each thigh | 0.4 | 1.1 | 0.4 |
| Each lower leg | 0.35 | 1.0 | 0.35 |

The torso's joint frame is at its center. Let `side = -1` for the character's
left and `+1` for its right. These are character-space names; an observer facing
the character may see left on the opposite side of the screen.

| Attachment | Position in its parent joint frame |
| --- | --- |
| Head base on torso | `(0, torso.height/2, 0)` |
| Shoulder on torso | `(side * (torso.width + upper_arm.width)/2, torso.height/2, 0)` |
| Elbow on upper arm | `(0, -upper_arm.length, 0)` |
| Hip on torso | `(side * torso.width/4, -torso.height/2, 0)` |
| Knee on thigh | `(0, -thigh.length, 0)` |

The head grows upward: from its base frame, offset its cube by
`T(0,head.height/2,0)` before scaling. Arms and legs grow downward.
Torso dimensions determine attachments; the torso's cube scaling is never
inherited by these frames.

## Keep dimensions and pose separate

Suggested data concepts:

- `BodyDimensions`: torso/head dimensions and dimensions for every limb segment.
- `BodyPose`: torso rotation, head rotation, left/right shoulder and hip angles,
  left/right elbow and knee angles.
- `RootTransform`: world position and facing/yaw of the whole person.

Use explicit fields or fixed arrays with named indices. Store degrees in the
pose to match scop's rotation API. Initially use X-axis shoulder/hip/elbow/knee
rotations for movement in the sagittal plane; torso yaw around Y demonstrates
inherited rotation clearly.
If multiple axes are added, document a fixed order, such as `Ry * Rx * Rz`.

## Traverse the hierarchy

The following pseudocode assumes every push succeeds; check the results in C++.

```text
draw_down_segment(dimensions):
    push
        multiply T(0, -dimensions.length/2, 0)
        multiply S(dimensions.width, dimensions.length, dimensions.depth)
        renderer.drawUnitCube(stack, part_color)
    pop

draw_two_part_limb(attachment, upper, lower, upper_angle, lower_angle):
    push
        multiply T(attachment)
        multiply Rx(upper_angle)
        draw_down_segment(upper)
        multiply T(0, -upper.length, 0)
        multiply Rx(lower_angle)
        draw_down_segment(lower)
    pop

draw_person(dimensions, pose, root):
    initialize stack
    load T(root.position) * Ry(root.yaw) * R(pose.torso)

    push
        multiply S(torso.width, torso.height, torso.depth)
        renderer.drawUnitCube(stack, torso_color)
    pop

    push
        multiply T(0, torso.height/2, 0)
        multiply R(pose.head)
        multiply T(0, head.height/2, 0)
        multiply S(head.width, head.height, head.depth)
        renderer.drawUnitCube(stack, head_color)
    pop

    draw_two_part_limb(left_shoulder, left_upper_arm, left_forearm,
                       pose.left_shoulder, pose.left_elbow)
    draw_two_part_limb(right_shoulder, right_upper_arm, right_forearm,
                       pose.right_shoulder, pose.right_elbow)
    draw_two_part_limb(left_hip, left_thigh, left_lower_leg,
                       pose.left_hip, pose.left_knee)
    draw_two_part_limb(right_hip, right_thigh, right_lower_leg,
                       pose.right_hip, pose.right_knee)

    assert stack count == 1
```

`R(pose.torso)` and `R(pose.head)` mean your selected rotation composition, not
an additional library. Attachments are calculated from the current dimensions.
The call count is `1 + 1 + 4 * 2 = 10`. Helper functions may invoke the shared
primitive function; each part still reaches it once. Keep debug markers or other
scene geometry out of that body count.

## Exercise and numerical checkpoint

`make test` now includes the two-part arm cases below, plus inherited 3D root
motion, resizing, isolated elbow movement, preserved forearm dimensions, unchanged
parent stack, and rejected invalid dimensions/angles. `make test-render` also
checks the visible upper-arm and forearm colors and exactly two indexed draws.

Build just a shoulder and two arm segments first. Set the shoulder at the origin,
upper length 2, lower length 1, and both angles to zero. The upper cube center is
`(0,-1,0)`, the elbow is `(0,-2,0)`, and the wrist is `(0,-3,0)`.

Rotate the shoulder +90° around Z. The elbow must move to `(2,0,0)` and wrist to
`(3,0,0)`. Then set the elbow angle to +90° too: the wrist must be `(2,1,0)`.
The elbow angle is relative to its parent, not an absolute world angle.

Extend that proven pattern to the full hierarchy. Torso yaw must move the head,
both arms, and both legs; a left-shoulder rotation must leave the right arm and
legs unchanged. Use different body colors while learning to distinguish branches.

## Research task, mistakes, and defense questions

Trace `world_torso * shoulder_attachment * shoulder_rotation * elbow_offset *
elbow_rotation` on paper and apply it to the forearm's local origin. Compare it
with the diagram in [Stage 1](01-assignment.md). This is your own worked research
example of hierarchical composition.

Independent world-space elbow positions break inheritance. Rotating the cube
around its center produces a spinning segment rather than a joint. Passing the
upper arm's geometry scale down to the forearm changes the forearm's dimensions.

Which matrix places the wrist? Why is a shoulder offset multiplied before its
rotation? Why can a non-drawn joint still be part of the hierarchy?
