#include "../src/scene/Arm.hpp"

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

Point pointAt(const MatrixStack::Matrix &matrix, GLfloat y) {
    return {matrix[1] * y + matrix[3], matrix[5] * y + matrix[7],
            matrix[9] * y + matrix[11]};
}

void expectPoint(const Point &actual, const Point &expected, const char *message) {
    for (int axis = 0; axis < 3; ++axis) {
        require(std::fabs(actual[axis] - expected[axis]) < 1e-5f, message);
    }
}

void expectConnected(const std::array<MatrixStack::Matrix, 2> &models) {
    expectPoint(pointAt(models[0], -0.5f), pointAt(models[1], 0.5f),
                "The upper-arm endpoint and forearm base must coincide");
}

GLfloat axisLength(const MatrixStack::Matrix &matrix, int axis) {
    return std::sqrt(matrix[axis] * matrix[axis] + matrix[4 + axis] * matrix[4 + axis]
                     + matrix[8 + axis] * matrix[8 + axis]);
}

void expectMirrored(const std::array<MatrixStack::Matrix, 4> &models) {
    for (int part = 0; part < 2; ++part) {
        for (GLfloat y : {-0.5f, 0.0f, 0.5f}) {
            const auto right = pointAt(models[part + 2], y);
            expectPoint(pointAt(models[part], y), {-right[0], right[1], right[2]},
                        "Arms must mirror across the parent's local X=0 plane");
        }
    }
    expectConnected({models[0], models[1]});
    expectConnected({models[2], models[3]});
}

void testPairedArms() {
    Arm shared;
    MatrixStack parent;
    const auto original = parent.top();
    shared.pose = {0.0f, 0.0f};
    auto models = shared.pairedModelMatrices(parent, 2.0f);
    expectPoint(pointAt(models[0], 0.5f), {-1.0f, 0.0f, 0.0f}, "Left shoulder spacing");
    expectPoint(pointAt(models[2], 0.5f), {1.0f, 0.0f, 0.0f}, "Right shoulder spacing");
    expectMirrored(models);

    shared.pose = {20.0f, 65.0f};
    models = shared.pairedModelMatrices(parent);
    expectMirrored(models);
    const auto beforeShoulder = models;
    shared.pose.shoulderDegrees += 10.0f;
    models = shared.pairedModelMatrices(parent);
    require(models[0] != beforeShoulder[0] && models[2] != beforeShoulder[2],
            "One shared shoulder angle must move both upper arms");
    expectMirrored(models);

    const auto beforeElbow = models;
    shared.pose.elbowDegrees += 10.0f;
    models = shared.pairedModelMatrices(parent);
    require(models[0] == beforeElbow[0] && models[2] == beforeElbow[2],
            "Shared elbow movement must preserve both upper arms");
    require(models[1] != beforeElbow[1] && models[3] != beforeElbow[3],
            "One shared elbow angle must move both forearms");
    expectMirrored(models);

    shared.dimensions.upper.length += 0.6f;
    models = shared.pairedModelMatrices(parent);
    expectMirrored(models);
    for (int forearm : {1, 3}) {
        require(std::fabs(axisLength(models[forearm], 1) - shared.dimensions.forearm.length) < 1e-5f,
                "Resizing upper arms must preserve both forearm lengths");
    }
    const auto beforeForearm = models;
    shared.dimensions.forearm.length += 0.4f;
    models = shared.pairedModelMatrices(parent);
    require(models[0] == beforeForearm[0] && models[2] == beforeForearm[2],
            "Shared forearm resizing must preserve both upper arms");
    expectMirrored(models);
    require(shared.pose.shoulderDegrees == 30.0f && shared.pose.elbowDegrees == 75.0f,
            "Mirroring must not change the shared control values");
    require(parent.top() == original && parent.size() == 1, "Paired arms preserve the parent");

    models = shared.pairedModelMatrices(parent, 3.0f);
    expectPoint(pointAt(models[0], 0.5f), {-1.5f, 0.0f, 0.0f}, "Changed left spacing");
    expectPoint(pointAt(models[2], 0.5f), {1.5f, 0.0f, 0.0f}, "Changed right spacing");

    shared = Arm{};
    shared.pose = {0.0f, 0.0f};
    parent.translate(5.0f, 3.0f, -4.0f);
    parent.rotateZ(90.0f);
    const auto root = parent.top();
    models = shared.pairedModelMatrices(parent, 2.0f);
    expectPoint(pointAt(models[0], 0.5f), {5.0f, 2.0f, -4.0f}, "Left shoulder inherits root rotation");
    expectPoint(pointAt(models[2], 0.5f), {5.0f, 4.0f, -4.0f}, "Right shoulder inherits root rotation");
    expectPoint(pointAt(models[1], -0.5f), {7.6f, 2.0f, -4.0f}, "Left wrist inherits root motion");
    expectPoint(pointAt(models[3], -0.5f), {7.6f, 4.0f, -4.0f}, "Right wrist inherits root motion");
    for (GLfloat bad : {0.0f, -1.0f, std::numeric_limits<GLfloat>::infinity(),
                        std::numeric_limits<GLfloat>::quiet_NaN()}) {
        bool rejected = false;
        try { shared.pairedModelMatrices(parent, bad); }
        catch (const std::invalid_argument &) { rejected = true; }
        require(rejected && parent.top() == root && parent.size() == 1,
                "Invalid spacing must preserve the parent");
    }
}
}

int main() {
    try {
        Arm arm;
        arm.pose = {0.0f, 0.0f};
        arm.dimensions.upper = {0.4f, 2.0f, 0.4f};
        arm.dimensions.forearm = {0.3f, 1.0f, 0.3f};
        MatrixStack parent;
        const auto original = parent.top();
        auto models = arm.modelMatrices(parent);
        expectPoint(pointAt(models[0], 0.0f), {0.0f, -1.0f, 0.0f}, "Upper-arm center");
        expectPoint(pointAt(models[0], 0.5f), {0.0f, 0.0f, 0.0f}, "Shoulder pivot");
        expectPoint(pointAt(models[1], -0.5f), {0.0f, -3.0f, 0.0f}, "Straight-arm wrist");
        expectConnected(models);
        require(parent.top() == original && parent.size() == 1, "Evaluating an arm preserves its parent");

        arm.pose.shoulderDegrees = 90.0f;
        models = arm.modelMatrices(parent);
        expectPoint(pointAt(models[0], -0.5f), {2.0f, 0.0f, 0.0f}, "Shoulder rotation carries elbow");
        expectPoint(pointAt(models[1], -0.5f), {3.0f, 0.0f, 0.0f}, "Shoulder rotation carries forearm");
        const auto upperBeforeElbow = models[0];
        arm.pose.elbowDegrees = 90.0f;
        models = arm.modelMatrices(parent);
        require(models[0] == upperBeforeElbow, "Elbow rotation must not move the upper arm");
        expectPoint(pointAt(models[1], -0.5f), {2.0f, 1.0f, 0.0f}, "Elbow rotation is parent-relative");
        expectConnected(models);

        arm.dimensions.upper.length = 3.0f;
        models = arm.modelMatrices(parent);
        expectPoint(pointAt(models[0], -0.5f), {3.0f, 0.0f, 0.0f}, "Longer upper arm repositions elbow");
        expectPoint(pointAt(models[1], -0.5f), {3.0f, 1.0f, 0.0f}, "Forearm follows resized upper arm");
        require(std::fabs(axisLength(models[1], 1) - 1.0f) < 1e-5f, "Upper-arm scaling must not stretch forearm");
        require(std::fabs(axisLength(models[1], 0) - 0.3f) < 1e-5f, "Forearm keeps its own width");
        expectConnected(models);

        const auto forearmBeforeWidth = models[1];
        arm.dimensions.upper.width = 1.0f;
        models = arm.modelMatrices(parent);
        require(models[1] == forearmBeforeWidth, "Parent drawing width must not move or scale the forearm");
        const auto upperBeforeLength = models[0];
        arm.dimensions.forearm.length = 2.0f;
        models = arm.modelMatrices(parent);
        require(models[0] == upperBeforeLength, "Resizing forearm must not change upper arm");
        expectPoint(pointAt(models[1], -0.5f), {3.0f, 2.0f, 0.0f}, "Forearm grows from elbow");
        expectConnected(models);

        parent.translate(5.0f, 6.0f, -4.0f);
        require(parent.push(), "Save attachment frame");
        const auto attachment = parent.top();
        models = arm.modelMatrices(parent);
        expectPoint(pointAt(models[0], 0.5f), {5.0f, 6.0f, -4.0f}, "Inherited shoulder position");
        expectPoint(pointAt(models[1], -0.5f), {8.0f, 8.0f, -4.0f}, "Inherited root translation");
        require(parent.top() == attachment && parent.size() == 2, "Parent depth and frame are unchanged");

        parent.reset();
        parent.translate(5.0f, 6.0f, -4.0f);
        parent.rotateX(90.0f);
        arm.pose = {0.0f, 0.0f};
        arm.dimensions.upper.length = 2.0f;
        arm.dimensions.forearm.length = 1.0f;
        models = arm.modelMatrices(parent);
        expectPoint(pointAt(models[1], -0.5f), {5.0f, 6.0f, -7.0f}, "Inherited 3D parent rotation");
        expectConnected(models);

        const auto beforeInvalid = parent.top();
        for (GLfloat bad : {0.0f, -1.0f, std::numeric_limits<GLfloat>::infinity(),
                            std::numeric_limits<GLfloat>::quiet_NaN()}) {
            Arm invalid = arm;
            invalid.dimensions.forearm.length = bad;
            bool rejected = false;
            try { invalid.modelMatrices(parent); }
            catch (const std::invalid_argument &) { rejected = true; }
            require(rejected && parent.top() == beforeInvalid && parent.size() == 1,
                    "Invalid dimensions must not alter the parent");
        }
        arm.pose.elbowDegrees = std::numeric_limits<GLfloat>::quiet_NaN();
        bool rejected = false;
        try { arm.modelMatrices(parent); }
        catch (const std::invalid_argument &) { rejected = true; }
        require(rejected, "Invalid joint angles are rejected");

        arm = Arm{};
        while (parent.size() < MatrixStack::capacity) require(parent.push(), "Fill parent stack");
        const auto full = parent.top();
        rejected = false;
        try { arm.modelMatrices(parent); }
        catch (const std::overflow_error &) { rejected = true; }
        require(rejected && parent.top() == full && parent.size() == MatrixStack::capacity,
                "An arm must not corrupt a full parent stack");
        testPairedArms();
        std::cout << "Passed " << checks << " single/mirrored-arm assertions.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Arm test failed: " << error.what() << '\n';
        return 1;
    }
}
