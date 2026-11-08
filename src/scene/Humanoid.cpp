#include "Humanoid.hpp"

#include <cmath>
#include <stdexcept>

std::array<MatrixStack::Matrix, 10> Humanoid::modelMatrices(const MatrixStack &parent) const {
    if (!std::isfinite(torso.width) || !std::isfinite(torso.length)
        || !std::isfinite(torso.depth) || torso.width <= 0.0f
        || torso.length <= 0.0f || torso.depth <= 0.0f) {
        throw std::invalid_argument("Torso dimensions must be finite and positive");
    }
    if (!std::isfinite(head.width) || !std::isfinite(head.length)
        || !std::isfinite(head.depth) || head.width <= 0.0f
        || head.length <= 0.0f || head.depth <= 0.0f) {
        throw std::invalid_argument("Head dimensions must be finite and positive");
    }

    MatrixStack body = parent;
    body.scale(torso.width, torso.length, torso.depth);

    MatrixStack headModel = parent;
    headModel.translate(0.0f, (torso.length + head.length) / 2.0f, 0.0f);
    headModel.scale(head.width, head.length, head.depth);

    std::array<MatrixStack::Matrix, 10> models;
    models[0] = body.top();
    models[9] = headModel.top();
    const std::array<ArmPose, 4> poses = {pose.leftArm, pose.rightArm, pose.leftLeg, pose.rightLeg};
    for (std::size_t index = 0; index < poses.size(); ++index) {
        const bool isArm = index < 2;
        const GLfloat side = index % 2 == 0 ? -1.0f : 1.0f;
        const GLfloat offset = isArm ? (torso.width + limbDimensions.upper.width) / 2.0f
                                     : torso.width / 4.0f;
        MatrixStack attachment = parent;
        attachment.translate(side * offset, (isArm ? 1.0f : -1.0f) * torso.length / 2.0f, 0.0f);
        Arm limb;
        limb.dimensions = limbDimensions;
        limb.pose = poses[index];
        const auto segments = limb.modelMatrices(attachment);
        models[index * 2 + 1] = segments[0];
        models[index * 2 + 2] = segments[1];
    }
    return models;
}
