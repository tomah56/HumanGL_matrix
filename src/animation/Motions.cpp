#include "Motions.hpp"

namespace animation {
const Sequence &walkingSequence() {
    static const Sequence walk = repeat({
        step(0.4, {
            to(Joint::LeftHip, 25), to(Joint::RightHip, -25),
            to(Joint::LeftKnee, -30), to(Joint::RightKnee, 0),
            to(Joint::LeftShoulder, -20), to(Joint::RightShoulder, 20),
            to(Joint::LeftElbow, 10), to(Joint::RightElbow, 25)
        }),
        step(0.4, {
            to(Joint::LeftHip, -25), to(Joint::RightHip, 25),
            to(Joint::LeftKnee, 0), to(Joint::RightKnee, -30),
            to(Joint::LeftShoulder, 20), to(Joint::RightShoulder, -20),
            to(Joint::LeftElbow, 25), to(Joint::RightElbow, 10)
        })
    });
    return walk;
}

const Sequence &jumpingSequence() {
    static const Sequence jump = once({
        // Crouch, rise to the apex, land in a crouch, and stand up.
        step(0.18, {
            to(Joint::LeftHip, 35), to(Joint::RightHip, 35),
            to(Joint::LeftKnee, -70), to(Joint::RightKnee, -70),
            to(Joint::LeftShoulder, -15), to(Joint::RightShoulder, -15),
            to(Joint::LeftElbow, 25), to(Joint::RightElbow, 25)
        }),
        step(0.30, {
            to(Joint::LeftHip, 5), to(Joint::RightHip, 5),
            to(Joint::LeftKnee, -10), to(Joint::RightKnee, -10),
            to(Joint::LeftShoulder, -80), to(Joint::RightShoulder, -80),
            to(Joint::LeftElbow, 15), to(Joint::RightElbow, 15)
        }, 1.1f),
        step(0.30, {
            to(Joint::LeftHip, 35), to(Joint::RightHip, 35),
            to(Joint::LeftKnee, -70), to(Joint::RightKnee, -70),
            to(Joint::LeftShoulder, -15), to(Joint::RightShoulder, -15),
            to(Joint::LeftElbow, 25), to(Joint::RightElbow, 25)
        }),
        step(0.18, {
            to(Joint::LeftHip, 0), to(Joint::RightHip, 0),
            to(Joint::LeftKnee, 0), to(Joint::RightKnee, 0),
            to(Joint::LeftShoulder, 0), to(Joint::RightShoulder, 0),
            to(Joint::LeftElbow, 0), to(Joint::RightElbow, 0)
        })
    });
    return jump;
}

void standStill(Humanoid &body) {
    for (int joint = 0; joint < static_cast<int>(Joint::Count); ++joint) {
        body.setJointAngle(static_cast<Joint>(joint), 0.0f);
    }
}
}
