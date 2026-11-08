#pragma once

#include "Arm.hpp"

struct HumanoidPose {
    ArmPose leftArm;
    ArmPose rightArm;
    // For legs, shoulderDegrees is the hip angle and elbowDegrees is the knee.
    ArmPose leftLeg;
    ArmPose rightLeg;
};

class Humanoid {
public:
    // The torso uses length as its height; its joint frame is at its center.
    SegmentDimensions torso = {1.2f, 1.8f, 0.6f};
    SegmentDimensions head = {0.6f, 0.7f, 0.6f};
    ArmDimensions limbDimensions;
    HumanoidPose pose;

    // Torso, left arm, right arm, left leg, right leg (two models per limb), head.
    // Geometry scaling stays out of the attachment frames.
    std::array<MatrixStack::Matrix, 10> modelMatrices(const MatrixStack &parent) const;
};
