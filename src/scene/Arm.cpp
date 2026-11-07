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
    joints.rotateZ(pose.shoulderDegrees);
    const auto upper = segmentModel(joints, dimensions.upper);

    // The geometry scale was popped: the elbow inherits the shoulder rotation
    // and upper-arm length, while the forearm retains its own dimensions.
    joints.translate(0.0f, -dimensions.upper.length, 0.0f);
    joints.rotateZ(pose.elbowDegrees);
    const auto forearm = segmentModel(joints, dimensions.forearm);
    return {upper, forearm};
}
