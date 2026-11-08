#include "../src/scene/Humanoid.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
using Point = std::array<GLfloat, 3>;
int checks = 0;

void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
    ++checks;
}

Point pointAt(const MatrixStack::Matrix &matrix, Point point) {
    Point result{};
    for (int row = 0; row < 3; ++row) {
        result[row] = matrix[row * 4 + 3];
        for (int column = 0; column < 3; ++column) {
            result[row] += matrix[row * 4 + column] * point[column];
        }
    }
    return result;
}

void expectPoint(Point actual, Point expected, const char *message) {
    for (int axis = 0; axis < 3; ++axis) {
        require(std::fabs(actual[axis] - expected[axis]) < 1e-5f, message);
    }
}

GLfloat axisLength(const MatrixStack::Matrix &matrix, int axis) {
    return std::sqrt(matrix[axis] * matrix[axis] + matrix[4 + axis] * matrix[4 + axis]
                     + matrix[8 + axis] * matrix[8 + axis]);
}

void testSidewaysArmJoints() {
    Humanoid body;
    body.pose = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
    MatrixStack parent;
    const auto original = parent.top();
    const auto rest = body.modelMatrices(parent);
    body.setJointAngle(Joint::RightShoulderSide, 90);
    body.setJointAngle(Joint::RightElbowSide, 90);
    auto models = body.modelMatrices(parent);
    expectPoint(pointAt(models[3], {0, -0.5f, 0}), {2.21f, 0.9f, 0}, "Sideways shoulder raises upper arm");
    expectPoint(pointAt(models[4], {0, 0.5f, 0}), {2.21f, 0.9f, 0}, "Sideways elbow stays connected");
    expectPoint(pointAt(models[4], {0, -0.5f, 0}), {2.21f, 2.1f, 0}, "Sideways forearm points upward");
    for (int part : {0, 1, 2, 5, 6, 7, 8, 9}) {
        require(models[part] == rest[part], "Sideways arm motion preserves all other branches");
    }
    const auto upper = models[3];
    const auto forearm = models[4];
    body.moveJoint(Joint::RightElbowSide, -45);
    models = body.modelMatrices(parent);
    require(models[3] == upper && models[4] != forearm, "Sideways elbow movement changes only forearm");
    require(parent.top() == original && parent.size() == 1, "Sideways joints preserve parent stack");
    for (bool shoulder : {false, true}) {
        Humanoid invalid = body;
        (shoulder ? invalid.pose.rightArm.shoulderSideDegrees : invalid.pose.rightArm.elbowSideDegrees)
            = std::numeric_limits<GLfloat>::quiet_NaN();
        bool rejected = false;
        try { invalid.modelMatrices(parent); }
        catch (const std::invalid_argument &) { rejected = true; }
        require(rejected && parent.top() == original, "Reject nonfinite sideways joint angles");
    }
}
}

int main() {
    try {
        Humanoid body;
        body.pose = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
        MatrixStack parent;
        const auto original = parent.top();
        auto models = body.modelMatrices(parent);
        expectPoint(pointAt(models[0], {0, 0, 0}), {0, 0, 0}, "Torso centered at root");
        expectPoint(pointAt(models[9], {0, 0, 0}), {0, 1.25f, 0}, "Head centered above torso");
        expectPoint(pointAt(models[9], {0, -0.5f, 0}),
                    pointAt(models[0], {0, 0.5f, 0}), "Head bottom touches torso top");
        expectPoint(pointAt(models[1], {0, 0.5f, 0}), {-0.81f, 0.9f, 0}, "Left shoulder");
        expectPoint(pointAt(models[3], {0, 0.5f, 0}), {0.81f, 0.9f, 0}, "Right shoulder");
        expectPoint(pointAt(models[1], {0.5f, 0.5f, 0}), {-0.6f, 0.9f, 0}, "Left arm touches side");
        expectPoint(pointAt(models[3], {-0.5f, 0.5f, 0}), {0.6f, 0.9f, 0}, "Right arm touches side");
        expectPoint(pointAt(models[5], {0, 0.5f, 0}), {-0.3f, -0.9f, 0}, "Left hip below torso");
        expectPoint(pointAt(models[7], {0, 0.5f, 0}), {0.3f, -0.9f, 0}, "Right hip below torso");
        for (int upper : {1, 3, 5, 7}) {
            expectPoint(pointAt(models[upper], {0, -0.5f, 0}),
                        pointAt(models[upper + 1], {0, 0.5f, 0}), "Connected elbow/knee");
        }
        require(parent.top() == original && parent.size() == 1, "Body preserves parent stack");

        body.torso = {2.0f, 2.4f, 0.8f};
        models = body.modelMatrices(parent);
        expectPoint(pointAt(models[9], {0, -0.5f, 0}), {0, 1.2f, 0}, "Head follows taller torso");
        for (int axis = 0; axis < 3; ++axis) {
            const GLfloat expected = axis == 1 ? 0.7f : 0.6f;
            require(std::fabs(axisLength(models[9], axis) - expected) < 1e-5f,
                    "Torso resizing preserves head dimensions");
        }
        expectPoint(pointAt(models[1], {0, 0.5f, 0}), {-1.21f, 1.2f, 0}, "Wider/taller left shoulder");
        expectPoint(pointAt(models[3], {0, 0.5f, 0}), {1.21f, 1.2f, 0}, "Wider/taller right shoulder");
        expectPoint(pointAt(models[5], {0, 0.5f, 0}), {-0.5f, -1.2f, 0}, "Resized left hip");
        expectPoint(pointAt(models[7], {0, 0.5f, 0}), {0.5f, -1.2f, 0}, "Resized right hip");
        for (int upper : {1, 3, 5, 7}) {
            require(std::fabs(axisLength(models[upper], 1) - 1.4f) < 1e-5f,
                    "Torso scaling must preserve limb lengths");
            require(std::fabs(axisLength(models[upper], 0) - 0.42f) < 1e-5f,
                    "Torso scaling must preserve limb widths");
        }
        body.limbDimensions.upper.width = 0.8f;
        models = body.modelMatrices(parent);
        expectPoint(pointAt(models[1], {0.5f, 0.5f, 0}), {-1.0f, 1.2f, 0}, "Resized arm stays at torso side");

        const auto beforeHeadResize = models;
        body.head = {0.8f, 1.0f, 0.9f};
        models = body.modelMatrices(parent);
        expectPoint(pointAt(models[9], {0, -0.5f, 0}), {0, 1.2f, 0}, "Resized head stays attached");
        expectPoint(pointAt(models[9], {0, 0.5f, 0}), {0, 2.2f, 0}, "Head grows upward from torso");
        for (int part = 0; part < 9; ++part) {
            require(models[part] == beforeHeadResize[part], "Head resizing preserves torso and limbs");
        }

        body = Humanoid{};
        const auto rest = body.modelMatrices(parent);
        body.pose = {{55, 35}, {55, 35}, {55, 35}, {55, 35}};
        body.limbDimensions.upper.length = 2.0f;
        models = body.modelMatrices(parent);
        require(models[0] == rest[0], "Joint movement and limb resizing preserve torso");
        require(models[9] == rest[9], "Joint movement and limb resizing preserve head");
        for (int upper : {1, 3, 5, 7}) {
            expectPoint(pointAt(models[upper], {0, 0.5f, 0}),
                        pointAt(rest[upper], {0, 0.5f, 0}), "Shoulder/hip remains attached during movement");
            expectPoint(pointAt(models[upper], {0, -0.5f, 0}),
                        pointAt(models[upper + 1], {0, 0.5f, 0}), "Moving/resized elbow/knee stays connected");
            require(std::fabs(axisLength(models[upper + 1], 1) - 1.2f) < 1e-5f,
                    "Lower segments keep their lengths after resizing upper segments");
            for (int entry : {0, 1, 2, 4, 5, 6, 8, 9, 10}) {
                require(std::fabs(models[upper][entry] - models[1][entry]) < 1e-5f,
                        "Equal poses retain identical forward rotations");
            }
        }

        body = Humanoid{};
        body.pose = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
        parent.translate(5.0f, 3.0f, -4.0f);
        parent.rotateX(90.0f);
        const auto root = parent.top();
        models = body.modelMatrices(parent);
        expectPoint(pointAt(models[0], {0, 0, 0}), {5, 3, -4}, "Torso inherits root");
        expectPoint(pointAt(models[9], {0, 0, 0}), {5, 3, -2.75f}, "Head inherits torso rotation");
        expectPoint(pointAt(models[1], {0, 0.5f, 0}), {4.19f, 3, -3.1f}, "Shoulder inherits torso rotation");
        expectPoint(pointAt(models[5], {0, 0.5f, 0}), {4.7f, 3, -4.9f}, "Hip inherits torso rotation");
        expectPoint(pointAt(models[8], {0, -0.5f, 0}), {5.3f, 3, -7.5f}, "Leg endpoint inherits torso rotation");
        require(parent.top() == root && parent.size() == 1, "Rotated body preserves parent");

        for (GLfloat bad : {0.0f, -1.0f, std::numeric_limits<GLfloat>::infinity(),
                            std::numeric_limits<GLfloat>::quiet_NaN()}) {
            for (GLfloat *dimension : {&body.torso.width, &body.torso.length, &body.torso.depth,
                                      &body.head.width, &body.head.length, &body.head.depth}) {
                const GLfloat previous = *dimension;
                *dimension = bad;
                bool rejected = false;
                try { body.modelMatrices(parent); }
                catch (const std::invalid_argument &) { rejected = true; }
                require(rejected && parent.top() == root, "Reject invalid body dimensions without changing parent");
                *dimension = previous;
            }
        }
        testSidewaysArmJoints();
        std::cout << "Passed " << checks << " body/attachment hierarchy assertions.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Humanoid test failed: " << error.what() << '\n';
        return 1;
    }
}
