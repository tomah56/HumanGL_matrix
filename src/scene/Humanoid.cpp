#include "Humanoid.hpp"

#include <cmath>
#include <stdexcept>

std::array<MatrixStack::Matrix, 9> Humanoid::modelMatrices(const MatrixStack &parent) const {
    if (!std::isfinite(torso.width) || !std::isfinite(torso.length)
        || !std::isfinite(torso.depth) || torso.width <= 0.0f
        || torso.length <= 0.0f || torso.depth <= 0.0f) {
        throw std::invalid_argument("Torso dimensions must be finite and positive");
    }

    MatrixStack body = parent;
    body.scale(torso.width, torso.length, torso.depth);

    MatrixStack shoulders = parent;
    shoulders.translate(0.0f, torso.length / 2.0f, 0.0f);
    const auto arms = limbs.pairedModelMatrices(shoulders, torso.width + limbs.dimensions.upper.width);

    MatrixStack hips = parent;
    hips.translate(0.0f, -torso.length / 2.0f, 0.0f);
    const auto legs = limbs.pairedModelMatrices(hips, torso.width / 2.0f);

    std::array<MatrixStack::Matrix, 9> models;
    models[0] = body.top();
    for (std::size_t part = 0; part < arms.size(); ++part) {
        models[part + 1] = arms[part];
        models[part + 5] = legs[part];
    }
    return models;
}
