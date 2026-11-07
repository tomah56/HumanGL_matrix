# 8. Animate idle, walking, and jumping

[Previous: proportions](07-proportions.md) · [Guide index](README.md) · [Next: validation](09-validation-and-defense.md)

## What you are learning

Animation changes the pose and root transform over time. The hierarchy then
calculates the body matrices exactly as it does for a still pose. Keep time and
pose evaluation separate from drawing; the draw function should not advance
angles or decide which animation is active.

Use seconds from [GLFW time input](https://www.glfw.org/docs/latest/input#time).
For each frame, `dt = now - previous` and then `previous = now`. Initialize
`previous` before the loop. A quantity updated by `speed * dt` has a speed in
units per second, rather than units per frame.

## Establish simple controls and state

Suggested controls for the first complete version:

| Input | Behavior |
| --- | --- |
| `1` | Select idle. |
| `2` | Select walking. |
| Space, on a new press | Start a jump if grounded. |
| `P`, on a new press | Pause/resume animation time. |
| `R`, on a new press | Restore idle, default pose, root position, and animation clocks. |
| Escape | Close the application. |

Use `AnimationState` with current mode `IDLE`, `WALK`, or `JUMP`, a requested
ground mode `IDLE`/`WALK`, walk phase, and jump elapsed time. During a jump, keys
1/2 choose the mode to enter after landing. Ignore repeated jump requests while
airborne. Edge-triggered controls must ignore keyboard repeat events. Scop already
defines an `AnimationState` for automatic viewer rotation; replace or rename that
state when adding this controller so the two responsibilities do not collide.

Pausing freezes animation clocks and root movement. Continue event processing
and refresh `previous` every frame so resuming does not accumulate paused time.

## Idle first

Set joint angles to the rest pose, keep the root fixed, and set the jump offset to
zero. Keep this version truly stationary so it clearly demonstrates “stay put.”
Camera movement, if provided later, should not modify the body's pose.

## Construct a walk cycle

Start by walking in place. Let cycle frequency be `f = 1.2` cycles/second and
advance phase by `2*pi*f*dt`, wrapping it into `[0,2*pi)`. Evaluate the pose
from phase each frame; do not keep adding small angles to the previous pose.

For a starting gait, define `s = sin(phase)`, hip amplitude `A=25°`, shoulder
amplitude `B=20°`, and knee bend `K=35°`. Keep the amplitudes and resulting joint
angles in degrees for scop's rotation API; only the sine phase is in radians.

```text
left_hip       = -A * s
right_hip      =  A * s
left_shoulder  =  B * s
right_shoulder = -B * s
left_knee      =  K * max(0,  s)
right_knee     =  K * max(0, -s)
```

These angles use the guide's convention: limbs point down −Y, facing is +Z,
and positive X rotation sends a downward limb toward −Z. At positive sine,
the left thigh swings forward and the right arm swings forward. A positive
knee angle bends the lower leg backward. Keep elbows/head/torso at rest for the
first cycle, then add small coordinated motions if helpful.

This gives a readable procedural gait. It does not guarantee perfectly planted
feet. Verify joints stay connected before tuning realism. For this first gait,
the straight-leg resting height prevents the endpoints penetrating a flat floor
with an upright torso; the bent/swinging legs can rise above it.

After the in-place cycle works, advance the root in its facing direction. For
yaw converted to radians (`theta = degreesToRadians(root.yaw)`) and speed
`v=1.0` world unit/second:

```text
root.x += sin(theta) * v * dt
root.z += cos(theta) * v * dt
```

An easy way to keep the person visible is a follow camera: target the torso root,
and set the eye to that root plus a fixed offset such as `(5,2,8)`, using your
look-at matrix. Reset restores the original position. Treat the in-place cycle
as a debugging milestone, then demonstrate actual travel in the complete version.

## Construct a jump

Use an analytic vertical arc with initial velocity `v0=4` units/second and
gravity magnitude `g=9.8` units/second²:

```text
duration = 2*v0/g
height(t) = v0*t - 0.5*g*t*t, for 0 <= t <= duration
```

Its duration is about 0.81633 seconds; its maximum height is about 0.81633 units
at `t=v0/g`, about 0.40816 seconds. Add the height to the dimension-derived
resting root Y; never add it to the previous frame's Y.

```text
on new grounded jump press:
    remember requested ground mode
    mode = JUMP
    jump_elapsed = 0

update(dt):
    if paused: return
    start pose from the rest pose
    if mode == IDLE:
        jump_offset = 0
    else if mode == WALK:
        advance phase, evaluate gait, advance horizontal root
        jump_offset = 0
    else if mode == JUMP:
        jump_elapsed += dt
        if jump_elapsed >= duration:
            jump_offset = 0
            mode = requested ground mode
        else:
            jump_offset = v0*jump_elapsed - 0.5*g*jump_elapsed^2
    root.y = resting_root_y + jump_offset
```

For this version a jump is vertical and horizontal travel pauses until landing;
the pose is straight during flight. Walking resumes on the next update if that
mode was requested. A later jumping pose can bend knees or raise arms without
changing the arc or hierarchy.

## Exercises and checkpoint

Evaluate the gait at phases `0`, `pi/2`, `pi`, and `3*pi/2`. At `pi/2` expect
left hip −25°, right hip +25°, left shoulder +20°, right shoulder −20°, left
knee +35°, right knee 0°. Check the opposite half-cycle too.

Simulate one second as 30, 60, and 144 equal updates without rendering. Starting
from yaw zero and walking, each simulation should move the root by +1 in Z and
reach the same wrapped phase within numerical tolerance. Sample the jump formula
at takeoff, apex, and landing. Simulate a step crossing landing time and confirm
the offset becomes exactly zero and the mode changes once.

Try pause mid-jump, repeated Space, selecting idle during a jump, resetting
mid-walk, and resizing symmetric legs. Refresh the resting height independently
of the jump offset. Your rendered checks should show continuous attachments
through every state; the clock must not depend on frame count.

## Research task, mistakes, and defense questions

Read GLFW's keyboard events and time-input sections. Explain why a new key press
differs from a held key. Sketch the three states and the transitions described
above. If a long stall causes a large step, the analytic jump still lands safely;
an optional simulation-step cap changes elapsed-time behavior and must be a
deliberate later decision.

Common bugs are degrees passed to sine, drift from incrementally rotating last
frame's matrix, adding jump height repeatedly, and immediately retriggering a jump
because Space is held. Rebuild matrices from dimensions and the evaluated pose
on every frame.

Why does `speed * dt` remove frame-rate dependence? How can the whole body jump
without changing each part's position separately? What happens to mode and height
when the next frame arrives after the landing time?
