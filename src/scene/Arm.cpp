#include "Arm.hpp"

#include <cmath>
#include <stdexcept>

namespace {
void validate(const SegmentDimensions &segment) {
    if (!std::isfinite(segment.width) || !std::isfinite(segment.length)
        || !std::isfinite(segment.depth) || segment.width <= 0.0f
        || segment.length <= 0.0f || segment.depth <= 0.0f) {
        throw std::invalid_argument("Arm dimensions must be finite and positive");
    }
}

MatrixStack::Matrix segmentModel(MatrixStack &joint, const SegmentDimensions &segment) {
    if (!joint.push()) throw std::overflow_error("Arm matrix stack overflow");
    joint.translate(0.0f, -segment.length / 2.0f, 0.0f);
    joint.scale(segment.width, segment.length, segment.depth);
    const auto model = joint.top();
    if (!joint.pop()) throw std::underflow_error("Arm matrix stack underflow");
    return model;
}
}

std::array<MatrixStack::Matrix, 2> Arm::modelMatrices(const MatrixStack &parent) const {
    validate(dimensions.upper);
    validate(dimensions.forearm);
    if (!std::isfinite(pose.shoulderDegrees) || !std::isfinite(pose.elbowDegrees)) {
        throw std::invalid_argument("Arm joint angles must be finite");
    }

    MatrixStack joints = parent;
    joints.rotateX(-pose.shoulderDegrees);
    const auto upper = segmentModel(joints, dimensions.upper);

    // The geometry scale was popped: the elbow inherits the shoulder rotation
    // and upper-arm length, while the forearm retains its own dimensions.
    joints.translate(0.0f, -dimensions.upper.length, 0.0f);
    joints.rotateX(-pose.elbowDegrees);
    const auto forearm = segmentModel(joints, dimensions.forearm);
    return {upper, forearm};
}

std::array<MatrixStack::Matrix, 4> Arm::pairedModelMatrices(
    const MatrixStack &parent, GLfloat shoulderSpacing) const {
    if (!std::isfinite(shoulderSpacing) || shoulderSpacing <= 0.0f) {
        throw std::invalid_argument("Shoulder spacing must be finite and positive");
    }

    std::array<MatrixStack::Matrix, 4> models;
    for (int index = 0; index < 2; ++index) {
        const GLfloat side = index == 0 ? -1.0f : 1.0f;
        MatrixStack attachment = parent;
        attachment.translate(side * shoulderSpacing / 2.0f, 0.0f, 0.0f);
        const auto armModels = modelMatrices(attachment);
        models[index * 2] = armModels[0];
        models[index * 2 + 1] = armModels[1];
    }
    return models;
}

std::array<MatrixStack::Matrix, 8> Arm::limbModelMatrices(
    const MatrixStack &parent, GLfloat pairSpacing, GLfloat rowGap) const {
    if (!std::isfinite(rowGap) || rowGap < 0.0f) {
        throw std::invalid_argument("Limb row gap must be finite and nonnegative");
    }

    const auto arms = pairedModelMatrices(parent, pairSpacing);
    MatrixStack hips = parent;
    hips.translate(0.0f, -(dimensions.upper.length + dimensions.forearm.length + rowGap), 0.0f);
    const auto legs = pairedModelMatrices(hips, pairSpacing);
    std::array<MatrixStack::Matrix, 8> models;
    for (std::size_t part = 0; part < arms.size(); ++part) {
        models[part] = arms[part];
        models[part + arms.size()] = legs[part];
    }
    return models;
}
