#pragma once

#include "../matrixMath/MatrixStack.hpp"

#include <array>

struct SegmentDimensions {
    GLfloat width;
    GLfloat length;
    GLfloat depth;
};

struct ArmDimensions {
    SegmentDimensions upper = {0.42f, 1.4f, 0.42f};
    SegmentDimensions forearm = {0.34f, 1.2f, 0.34f};
};

struct ArmPose {
    GLfloat shoulderDegrees = 20.0f;
    GLfloat elbowDegrees = 65.0f;
};

class Arm {
public:
    ArmDimensions dimensions;
    ArmPose pose;

    // Upper-arm and forearm models, in that order. The parent is the shoulder
    // attachment frame. Joint rotations are about local Z for this first demo.
    std::array<MatrixStack::Matrix, 2> modelMatrices(const MatrixStack &parent) const;

    // Shared pose/dimensions for two arms. Shoulder attachments are spaced along
    // parent X; left joint angles are negated to mirror the right arm.
    // Order: left upper, left forearm, right upper, right forearm.
    std::array<MatrixStack::Matrix, 4> pairedModelMatrices(
        const MatrixStack &parent, GLfloat shoulderSpacing = 1.6f) const;
};
