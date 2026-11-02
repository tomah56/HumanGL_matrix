# 7. Make proportions adjustable

[Previous: hierarchy](06-body-hierarchy.md) · [Guide index](README.md) · [Next: animation](08-animation.md)

## What you are learning

Automatic repositioning follows from relationships. A longer upper arm moves the
elbow because the elbow attachment is calculated from that length. You should
not repair it by editing a second, unrelated position constant.

Keep dimensions in one easily edited configuration. Editing code and rebuilding
is sufficient for the assignment's defense instructions. Runtime size controls
can be added after the mandatory version works.

## Derive every dependent position

For a downward segment, length determines two different offsets:

```text
cube_center = (0, -length/2, 0)
next_joint  = (0, -length,   0)
```

Width and depth determine its cross-section. A changed width need not move the
elbow because this design connects segments along their center axes. Shoulder X
depends on torso width and upper-arm width. Head base Y and hip Y depend on torso
height. Hip X depends on torso width. Use the attachment formulas from
[Stage 6](06-body-hierarchy.md) every time you traverse the body.

Changing a forearm's length moves its free end; it does not move the elbow back
into the upper arm. Changing thigh length moves the knee and entire lower leg;
changing lower-leg length moves its lower endpoint.

## Keep a resting model above the ground

With symmetric straight legs and the torso frame at its center:

```text
resting_root_y = torso.height/2 + thigh.length + lower_leg.length
root_y = resting_root_y + jump_height_offset
```

For the default dimensions, `resting_root_y = 0.9 + 1.1 + 1.0 = 3.0`.
The lower-leg endpoints then lie at ground Y=0. Recompute the resting height
when dimensions change; do not store an unrelated constant that becomes stale.

If you permit different left/right leg lengths, use the larger sum for resting
height to keep the straight legs above the ground. The shorter leg will hang
above it. Keeping both unequal legs planted would require adjusting their pose;
that is an optional inverse-kinematics topic. The mandatory connection rule still
applies to every knee regardless of symmetry.

For a clear defense, start with symmetric legs and test one arm independently.
Keep the camera far enough away, or recenter it using the changed dimensions,
so a successful resize is not mistaken for a disappearing model.

## Exercise and expected results

Start from the isolated unrotated arm in Stage 6:

| Change | Expected elbow | Expected wrist |
| --- | --- | --- |
| Upper length 2, forearm length 1 | `(0,-2,0)` | `(0,-3,0)` |
| Upper length becomes 3, forearm remains 1 | `(0,-3,0)` | `(0,-4,0)` |
| Forearm length then becomes 2 | `(0,-3,0)` | `(0,-5,0)` |
| Upper width changes, lengths unchanged | Same as previous row | Same as previous row |

Repeat the length change with a rotated shoulder. The elbow must move along the
upper arm's rotated local axis, not along a fixed world Y axis.

For the complete body:

1. Increase torso height from 1.8 to 2.4. Relative head/shoulder attachments move
   up 0.3, hip attachments move down 0.3, and resting root height rises by 0.3.
2. Increase both thigh lengths from 1.1 to 1.6. Knees move 0.5 farther from the
   hips and resting root height rises by 0.5.
3. Increase torso width from 1.2 to 1.6. Each shoulder moves outward 0.2 and each
   hip moves outward 0.1, in the torso frame.
4. Restore defaults and repeat while walking. Connections should remain intact.

For CPU checks, compute a parent segment's distal endpoint and the child joint's
world position independently from the traversal outputs. Their distance should
be near zero before and after resizing. Also verify the forearm's own length
stays unchanged when only the upper arm grows.

## Research task and checkpoint

Create a dependency sketch: `upper length -> cube center + elbow offset`,
`torso height -> head/shoulder/hip attachments + resting height`. Search your
future code for duplicated position constants and replace each only when it
represents a relationship in that sketch.

You are ready when changing dimensions in one configuration automatically
updates all affected attachments, including in a rotated pose. Require positive,
finite dimensions; an eventual runtime control should reject invalid values
instead of building a degenerate or mirrored model accidentally.

## Common mistakes and defense questions

Updating just the draw scale leaves the elbow in the old location. Inheriting
that scale stretches children. Editing world coordinates by hand fixes one pose
while breaking another. A runtime GUI is not needed to prove the size dependency.

What quantities depend on upper-arm length? What depends on width? Why does a
torso resize affect resting height? Where is the single source of dimensions?
