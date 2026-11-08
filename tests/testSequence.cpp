#include "../src/animation/Motions.hpp"
#include "../src/inputHandler/LimbControls.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace {
using namespace animation;
int checks = 0;

void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
    ++checks;
}

void near(double actual, double expected, const char *message) {
    require(std::fabs(actual - expected) < 1e-4, message);
}

void testInterpolation() {
    Humanoid body;
    standStill(body);
    SequencePlayer player;
    player.play(once({
        step(1, {to(Joint::LeftShoulder, 40), to(Joint::RightHip, -20)}, 0.8f),
        step(1, {to(Joint::LeftElbow, 60)})
    }), body);
    player.update(0, body);
    near(body.jointAngle(Joint::LeftShoulder), 0, "Starting playback does not snap the pose");
    player.update(0.25, body);
    near(body.jointAngle(Joint::LeftShoulder), 10, "Interpolate shoulder");
    near(body.jointAngle(Joint::RightHip), -5, "Interpolate multiple joints together");
    near(body.jointAngle(Joint::LeftElbow), 0, "Untargeted joint stays put");
    near(player.heightOffset(), 0.2, "Interpolate body height");
    player.update(1, body);
    near(body.jointAngle(Joint::LeftShoulder), 40, "Completed step reaches exact target");
    near(body.jointAngle(Joint::LeftElbow), 15, "Carry leftover time into next step");
    near(player.heightOffset(), 0.6, "Next step starts at previous height");
    player.update(10, body);
    near(body.jointAngle(Joint::LeftElbow), 60, "Large update completes one-shot sequence");
    near(player.heightOffset(), 0, "Final step returns to ground");
    require(!player.isPlaying(), "One-shot stops at end");
    MatrixStack parent;
    const auto finished = body.modelMatrices(parent);
    player.update(5, body);
    require(body.modelMatrices(parent) == finished, "Finished sequence holds its final pose");
    near(body.limbDimensions.upper.length, 1.4, "Animation preserves part dimensions");
}

void testLoopsAndFrameRate() {
    Humanoid body;
    standStill(body);
    SequencePlayer player;
    player.play(repeat({step(0.5, {to(Joint::LeftHip, 30)}),
                        step(0.5, {to(Joint::LeftHip, -30)})}), body);
    player.update(0.5, body);
    near(body.jointAngle(Joint::LeftHip), 30, "First loop target");
    player.update(0.5, body);
    near(body.jointAngle(Joint::LeftHip), -30, "Second loop target");
    player.update(0.25, body);
    near(body.jointAngle(Joint::LeftHip), 0, "Next cycle starts at last endpoint");
    player.update(1000000000.25, body);
    near(body.jointAngle(Joint::LeftHip), 30, "Skip complete cycles without drift or a long loop");
    require(player.isPlaying(), "Repeating sequence remains active");

    standStill(body);
    player.play(repeat({step(0.5, {to(Joint::LeftShoulder, 10)}),
                        step(0.5, {to(Joint::LeftElbow, 20)})}), body);
    player.update(1.25, body);
    near(body.jointAngle(Joint::LeftElbow), 20, "Omitted joints keep their previous-cycle values");

    Humanoid slow, fast;
    SequencePlayer slowPlayer, fastPlayer;
    slowPlayer.play(walkingSequence(), slow);
    fastPlayer.play(walkingSequence(), fast);
    slowPlayer.update(3.25, slow);
    for (int frame = 0; frame < 390; ++frame) fastPlayer.update(1.0 / 120.0, fast);
    for (int joint = 0; joint < static_cast<int>(Joint::Count); ++joint) {
        near(slow.jointAngle(static_cast<Joint>(joint)), fast.jointAngle(static_cast<Joint>(joint)),
             "Walking is independent of frame rate");
    }
    near(slowPlayer.heightOffset(), 0, "Walking stays at its starting height");
}

void testJumpAndManualControl() {
    Humanoid body;
    standStill(body);
    SequencePlayer player;
    player.play(jumpingSequence(), body);
    player.update(0.18, body);
    near(player.heightOffset(), 0, "Crouch stays on ground");
    near(body.jointAngle(Joint::LeftKnee), -70, "Jump crouches both knees");
    player.update(0.15, body);
    near(player.heightOffset(), 0.55, "Jump rises halfway");
    player.update(0.15, body);
    near(player.heightOffset(), 1.1, "Jump reaches apex");
    player.update(0.30, body);
    near(player.heightOffset(), 0, "Jump lands");
    player.update(0.20, body);
    require(!player.isPlaying(), "Jump finishes after standing up");
    for (int joint = 0; joint < static_cast<int>(Joint::Count); ++joint) {
        near(body.jointAngle(static_cast<Joint>(joint)), 0, "Jump ends in stationary standing pose");
    }
    player.play(jumpingSequence(), body);
    player.update(0.33, body);
    LimbControls controls;
    controls.handleKey(body, GLFW_KEY_L, GLFW_PRESS, 0, &player);
    require(player.isPlaying(), "Holding selector alone does not interrupt playback");
    const GLfloat before = body.jointAngle(Joint::LeftShoulder);
    controls.handleKey(body, GLFW_KEY_A, GLFW_PRESS, 0, &player);
    near(body.jointAngle(Joint::LeftShoulder), before + 5, "Manual and sequence controls share joint API");
    require(!player.isPlaying(), "Manual joint movement stops playback");
    near(player.heightOffset(), 0, "Stopping playback grounds the body");
    player.update(10, body);
    near(body.jointAngle(Joint::LeftShoulder), before + 5, "Stopped player preserves manual movement");
    player.play(walkingSequence(), body);
    controls.handleKey(body, GLFW_KEY_0, GLFW_PRESS, 0, &player);
    require(!player.isPlaying(), "Manual reset stops playback");
    near(body.jointAngle(Joint::RightElbow), 65, "Reset restores existing manual default pose");
    standStill(body);
    near(body.jointAngle(Joint::RightElbow), 0, "Stand-still action uses a straight resting pose");
}

void testValidation() {
    Humanoid body;
    SequencePlayer player;
    player.play(once({step(1, {to(Joint::LeftShoulder, 40)})}), body);
    player.update(0.25, body);
    const GLfloat angle = body.jointAngle(Joint::LeftShoulder);
    const double infinity = std::numeric_limits<double>::infinity();
    const GLfloat nan = std::numeric_limits<GLfloat>::quiet_NaN();
    const std::vector<Sequence> invalid = {
        once({}), once({step(0, {})}), once({step(-1, {})}), once({step(infinity, {})}),
        once({step(nan, {})}), once({step(1, {}, nan)}),
        once({step(1, {to(Joint::LeftHip, nan)})}),
        once({step(1, {to(Joint::Count, 5)})}),
        once({step(1, {to(Joint::LeftHip, 5), to(Joint::LeftHip, 10)})}),
        once({step(1e308, {}), step(1e308, {})})
    };
    for (const auto &sequence : invalid) {
        bool rejected = false;
        try { player.play(sequence, body); }
        catch (const std::invalid_argument &) { rejected = true; }
        require(rejected && player.isPlaying(), "Invalid sequence preserves running playback");
        near(body.jointAngle(Joint::LeftShoulder), angle, "Rejected sequence preserves pose");
    }
    for (double delta : {-1.0, infinity, static_cast<double>(nan)}) {
        bool rejected = false;
        try { player.update(delta, body); }
        catch (const std::invalid_argument &) { rejected = true; }
        require(rejected, "Reject invalid delta time");
        near(body.jointAngle(Joint::LeftShoulder), angle, "Rejected delta preserves pose");
    }
    player.update(0.25, body);
    near(body.jointAngle(Joint::LeftShoulder), 30, "Rejected inputs preserve playback progress");
    for (Joint joint : {Joint::LeftHip, Joint::Count}) {
        bool rejected = false;
        try { body.setJointAngle(joint, nan); }
        catch (const std::invalid_argument &) { rejected = true; }
        require(rejected, "Shared joint setter rejects invalid inputs");
    }
    bool rejected = false;
    try { body.moveJoint(Joint::Count, 5); }
    catch (const std::invalid_argument &) { rejected = true; }
    require(rejected, "Shared joint movement rejects unknown joints");
}
}

int main() {
    try {
        testInterpolation();
        testLoopsAndFrameRate();
        testJumpAndManualControl();
        testValidation();
        std::cout << "Passed " << checks << " sequence playback assertions.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Sequence test failed: " << error.what() << '\n';
        return 1;
    }
}
