#include "../src/inputHandler/LimbControls.hpp"
#include "../src/scene/Humanoid.hpp"

#include <iostream>
#include <stdexcept>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace {
int checks = 0;

void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
    ++checks;
}

void testJoint(int limb, bool lower) {
    Humanoid body;
    LimbControls controls;
    MatrixStack parent;
    const auto before = body.modelMatrices(parent);
    const bool left = limb % 2 == 0;
    const bool arm = limb < 2;
    const int sideKey = left ? GLFW_KEY_L : GLFW_KEY_R;
    const int lowerKey = left ? GLFW_KEY_K : GLFW_KEY_T;
    const int forward = arm ? GLFW_KEY_A : GLFW_KEY_Q;
    const int backward = arm ? GLFW_KEY_D : GLFW_KEY_E;
    controls.handleKey(body, sideKey, GLFW_PRESS, 0);
    if (lower) controls.handleKey(body, lowerKey, GLFW_PRESS, 0);
    require(controls.handleKey(body, forward, GLFW_PRESS, 0), "Movement key must be handled");
    const auto moved = body.modelMatrices(parent);
    const int upperPart = limb * 2 + 1;
    for (int part = 0; part < 10; ++part) {
        const bool shouldMove = part == upperPart + 1 || (!lower && part == upperPart);
        require((moved[part] != before[part]) == shouldMove,
                "Only the selected segment and its descendants may move");
    }
    const std::array<ArmPose, 4> poses = {body.pose.leftArm, body.pose.rightArm,
                                       body.pose.leftLeg, body.pose.rightLeg};
    require((lower ? poses[limb].elbowDegrees : poses[limb].shoulderDegrees)
            == (lower ? 70.0f : 25.0f), "Forward key increases positive-forward flexion");
    controls.handleKey(body, backward, GLFW_PRESS, 0);
    require(body.modelMatrices(parent) == before, "Backward key must reverse the forward movement");
    controls.handleKey(body, forward, GLFW_REPEAT, 0);
    require(body.modelMatrices(parent) == moved, "Held movement key repeats the selected joint");
    controls.handleKey(body, forward, GLFW_RELEASE, 0);
    require(body.modelMatrices(parent) == moved, "Releasing movement key must not move a joint");
    controls.handleKey(body, sideKey, GLFW_RELEASE, 0);
    controls.handleKey(body, forward, GLFW_PRESS, 0);
    require(body.modelMatrices(parent) == moved, "Side must remain held during movement");
    controls.handleKey(body, sideKey, GLFW_PRESS, 0);
    if (lower) {
        controls.handleKey(body, lowerKey, GLFW_RELEASE, 0);
        controls.handleKey(body, forward, GLFW_PRESS, 0);
        const std::array<ArmPose, 4> released = {body.pose.leftArm, body.pose.rightArm,
                                              body.pose.leftLeg, body.pose.rightLeg};
        require(released[limb].shoulderDegrees == 25.0f && released[limb].elbowDegrees == 70.0f,
                "Releasing K/T immediately switches back to the upper segment");
    }
}
}

int main() {
    try {
        for (int limb = 0; limb < 4; ++limb) {
            testJoint(limb, false);
            testJoint(limb, true);
        }
        Humanoid body;
        LimbControls controls;
        MatrixStack parent;
        const auto initial = body.modelMatrices(parent);
        controls.handleKey(body, GLFW_KEY_A, GLFW_PRESS, 0);
        controls.handleKey(body, GLFW_KEY_Q, GLFW_REPEAT, 0);
        require(body.modelMatrices(parent) == initial, "Movement without a held side does nothing");
        controls.handleKey(body, GLFW_KEY_L, GLFW_PRESS, 0);
        controls.handleKey(body, GLFW_KEY_R, GLFW_PRESS, 0);
        controls.handleKey(body, GLFW_KEY_A, GLFW_PRESS, 0);
        require(body.modelMatrices(parent) == initial, "Two held sides do not move multiple limbs");
        controls.handleKey(body, GLFW_KEY_R, GLFW_RELEASE, GLFW_MOD_CONTROL);
        for (int mods : {GLFW_MOD_CONTROL, GLFW_MOD_SUPER}) {
            require(!controls.handleKey(body, GLFW_KEY_A, GLFW_PRESS, mods), "Preserve Ctrl/Cmd shortcuts");
            require(body.modelMatrices(parent) == initial, "Ctrl/Cmd must not move a joint");
        }
        controls.handleKey(body, GLFW_KEY_A, GLFW_PRESS, 0);
        require(body.pose.leftArm.shoulderDegrees == 25.0f, "Side release works with modifiers held");

        const auto poseBeforeResize = body.modelMatrices(parent);
        controls.handleKey(body, GLFW_KEY_2, GLFW_PRESS, 0);
        controls.handleKey(body, GLFW_KEY_4, GLFW_PRESS, 0);
        require(body.limbDimensions.upper.length > 1.4f && body.limbDimensions.forearm.length > 1.2f,
                "Existing length controls remain shared");
        controls.handleKey(body, GLFW_KEY_1, GLFW_PRESS, 0);
        controls.handleKey(body, GLFW_KEY_3, GLFW_PRESS, 0);
        require(body.modelMatrices(parent) == poseBeforeResize, "Resizing back preserves independent poses");
        body.limbDimensions.upper.length = body.limbDimensions.forearm.length = 0.2f;
        controls.handleKey(body, GLFW_KEY_1, GLFW_REPEAT, 0);
        controls.handleKey(body, GLFW_KEY_3, GLFW_REPEAT, 0);
        require(body.limbDimensions.upper.length == 0.2f && body.limbDimensions.forearm.length == 0.2f,
                "Lengths retain their minimum");
        controls.handleKey(body, GLFW_KEY_0, GLFW_PRESS, 0);
        require(body.modelMatrices(parent) == initial, "Reset restores all four poses and shared dimensions");
        require(!controls.handleKey(body, GLFW_KEY_LEFT, GLFW_PRESS, 0), "Keep arrow rotation in existing handler");
        std::cout << "Passed " << checks << " independent limb-control assertions.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Limb controls test failed: " << error.what() << '\n';
        return 1;
    }
}
