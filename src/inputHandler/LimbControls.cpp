#include "LimbControls.hpp"
#include "../scene/Humanoid.hpp"
#include "../animation/Sequence.hpp"

#include <algorithm>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

bool LimbControls::handleKey(Humanoid &body, int key, int action, int mods,
                             animation::SequencePlayer *player) {
    const bool held = action != GLFW_RELEASE;
    switch (key) {
        case GLFW_KEY_L: _leftHeld = held; break;
        case GLFW_KEY_R: _rightHeld = held; break;
        case GLFW_KEY_K: _kHeld = held; break;
        case GLFW_KEY_T: _tHeld = held; break;
    }
    // Releases always update selector state, including with Ctrl/Cmd held.
    if (mods & (GLFW_MOD_CONTROL | GLFW_MOD_SUPER)) return false;
    if (key == GLFW_KEY_L || key == GLFW_KEY_R || key == GLFW_KEY_K || key == GLFW_KEY_T) return true;
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return false;

    auto &dimensions = body.limbDimensions;
    switch (key) {
        case GLFW_KEY_1: dimensions.upper.length = std::max(0.2f, dimensions.upper.length - 0.1f); return true;
        case GLFW_KEY_2: dimensions.upper.length += 0.1f; return true;
        case GLFW_KEY_3: dimensions.forearm.length = std::max(0.2f, dimensions.forearm.length - 0.1f); return true;
        case GLFW_KEY_4: dimensions.forearm.length += 0.1f; return true;
        case GLFW_KEY_0:
            if (player) player->stop();
            dimensions = ArmDimensions{};
            body.pose = HumanoidPose{};
            return true;
    }

    const bool arm = key == GLFW_KEY_A || key == GLFW_KEY_D;
    const bool leg = key == GLFW_KEY_Q || key == GLFW_KEY_E;
    if (!arm && !leg) return false;
    // Movement requires exactly one held side; there is no latched selection.
    if (_leftHeld == _rightHeld) return true;
    const bool lower = _leftHeld ? _kHeld : _tHeld;
    const Joint joint = arm
        ? (_leftHeld ? (lower ? Joint::LeftElbow : Joint::LeftShoulder)
                     : (lower ? Joint::RightElbow : Joint::RightShoulder))
        : (_leftHeld ? (lower ? Joint::LeftKnee : Joint::LeftHip)
                     : (lower ? Joint::RightKnee : Joint::RightHip));
    if (player) player->stop();
    body.moveJoint(joint, (key == GLFW_KEY_A || key == GLFW_KEY_Q) ? 5.0f : -5.0f);
    return true;
}
