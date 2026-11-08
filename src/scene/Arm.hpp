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
    static constexpr GLfloat defaultPairSpacing = 1.6f;
    static constexpr GLfloat defaultRowGap = 0.6f;

    ArmDimensions dimensions;
    ArmPose pose;

    // Upper-arm and forearm models, in that order. The parent is the shoulder
    // attachment frame. Positive joint angles bend forward along local +Z,
    // using negative right-handed X rotations.
    std::array<MatrixStack::Matrix, 2> modelMatrices(const MatrixStack &parent) const;

    // Shared pose/dimensions for two arms. Shoulder attachments are spaced along
    // parent X; both use the same joint angles and face local +Z.
    // Order: left upper, left forearm, right upper, right forearm.
    std::array<MatrixStack::Matrix, 4> pairedModelMatrices(
        const MatrixStack &parent, GLfloat shoulderSpacing = defaultPairSpacing) const;

    // Reuse the arm pair as legs below it, with the same dimensions and pose.
    // Arms occupy indices 0..3, legs 4..7; rowGap is the rest-pose clearance.
    std::array<MatrixStack::Matrix, 8> limbModelMatrices(
        const MatrixStack &parent, GLfloat pairSpacing = defaultPairSpacing,
        GLfloat rowGap = defaultRowGap) const;
};
