#pragma once

#include "Arm.hpp"

class Humanoid {
public:
    // The torso uses length as its height; its joint frame is at its center.
    SegmentDimensions torso = {1.2f, 1.8f, 0.6f};
    Arm limbs;

    // Torso, left arm, right arm, left leg, right leg (two models per limb).
    // Geometry scaling stays out of the attachment frames.
    std::array<MatrixStack::Matrix, 9> modelMatrices(const MatrixStack &parent) const;
};
