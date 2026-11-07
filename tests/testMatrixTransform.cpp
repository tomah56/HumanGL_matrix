#include "../src/matrixMath/MatrixTransform.hpp"
#include "../src/scene/Camera.hpp"
#include "../src/scene/Transformation.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

namespace {

using Point = std::array<GLfloat, 4>;
using Matrix = std::array<GLfloat, 16>;
int checks = 0;

void require(bool condition, const std::string &message) {
    if (!condition) throw std::runtime_error(message);
    ++checks;
}

Point applyPoint(const GLfloat *matrix, const Point &point) {
    Point result = {};
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            result[row] += matrix[row * 4 + column] * point[column];
        }
    }
    return result;
}

void expectPoint(const Point &actual, const Point &expected, const std::string &name) {
    for (std::size_t i = 0; i < actual.size(); ++i) {
        require(std::isfinite(actual[i]) && std::fabs(actual[i] - expected[i]) < 1e-5f,
                name + ": component " + std::to_string(i) + " expected "
                + std::to_string(expected[i]) + ", got " + std::to_string(actual[i]));
    }
}

void testPerspective() {
    Matrix projection;
    projection.fill(std::numeric_limits<GLfloat>::quiet_NaN());
    MatrixTransform::perspective(projection.data(), 90.0f, 2.0f, 1.0f, 9.0f);
    const Matrix expected = {
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.25f, -2.25f,
        0.0f, 0.0f, -1.0f, 0.0f,
    };
    for (std::size_t i = 0; i < projection.size(); ++i) {
        require(std::isfinite(projection[i])
                && std::fabs(projection[i] - expected[i]) < 1e-5f,
                "Perspective must replace every entry, including stale values");
    }
    expectPoint(applyPoint(projection.data(), {0.0f, 0.0f, -1.0f, 1.0f}),
                {0.0f, 0.0f, -1.0f, 1.0f}, "Near plane");
    expectPoint(applyPoint(projection.data(), {0.0f, 0.0f, -9.0f, 1.0f}),
                {0.0f, 0.0f, 9.0f, 9.0f}, "Far plane");
    const Point closer = applyPoint(projection.data(), {1.0f, 0.0f, -2.0f, 1.0f});
    const Point farther = applyPoint(projection.data(), {1.0f, 0.0f, -4.0f, 1.0f});
    require(std::fabs(closer[0] / closer[3] - 0.25f) < 1e-5f,
            "Perspective divide at distance 2");
    require(std::fabs(farther[0] / farther[3] - 0.125f) < 1e-5f,
            "Doubling distance halves projected X");

    // Rebuilding after a different projection must also clear all old entries.
    MatrixTransform::perspective(projection.data(), 60.0f, 1.0f, 0.1f, 100.0f);
    require(projection[0] == projection[5] && projection[15] == 0.0f,
            "Rebuilt square-aspect perspective");
}

void testInvalidProjection() {
    const GLfloat infinity = std::numeric_limits<GLfloat>::infinity();
    const GLfloat nan = std::numeric_limits<GLfloat>::quiet_NaN();
    const std::array<GLfloat, 4> invalid[] = {
        {0.0f, 1.0f, 0.1f, 100.0f}, {180.0f, 1.0f, 0.1f, 100.0f},
        {-1.0f, 1.0f, 0.1f, 100.0f}, {45.0f, 0.0f, 0.1f, 100.0f},
        {45.0f, -1.0f, 0.1f, 100.0f}, {45.0f, 1.0f, 0.0f, 100.0f},
        {45.0f, 1.0f, -1.0f, 100.0f}, {45.0f, 1.0f, 1.0f, 1.0f},
        {45.0f, 1.0f, 2.0f, 1.0f}, {infinity, 1.0f, 0.1f, 100.0f},
        {45.0f, infinity, 0.1f, 100.0f}, {nan, 1.0f, 0.1f, 100.0f},
        {45.0f, 1.0f, nan, 100.0f}, {45.0f, 1.0f, 0.1f, nan},
    };
    for (const auto &parameters : invalid) {
        Matrix matrix;
        matrix.fill(3.0f);
        const Matrix before = matrix;
        bool rejected = false;
        try {
            MatrixTransform::perspective(matrix.data(), parameters[0], parameters[1],
                                         parameters[2], parameters[3]);
        } catch (const MatrixTransformException &) {
            rejected = true;
        }
        require(rejected, "Invalid perspective parameters must throw");
        require(matrix == before, "Invalid perspective must preserve the output matrix");
    }
}

void testScale() {
    Matrix matrix;
    MatrixTransform::loadIdentity(matrix.data());
    MatrixTransform::rotateZ(matrix.data(), 90.0f);
    MatrixTransform::translate(matrix.data(), 5.0f, -2.0f, 7.0f);
    MatrixTransform::scale(matrix.data(), 2.0f, 3.0f, 4.0f);
    expectPoint(applyPoint(matrix.data(), {1.0f, 0.0f, 0.0f, 1.0f}),
                {5.0f, 0.0f, 7.0f, 1.0f}, "Rotated local X scale");
    expectPoint(applyPoint(matrix.data(), {0.0f, 1.0f, 0.0f, 1.0f}),
                {2.0f, -2.0f, 7.0f, 1.0f}, "Rotated local Y scale");
    expectPoint(applyPoint(matrix.data(), {0.0f, 0.0f, 1.0f, 1.0f}),
                {5.0f, -2.0f, 11.0f, 1.0f}, "Local Z scale");
    expectPoint(applyPoint(matrix.data(), {0.0f, 0.0f, 0.0f, 1.0f}),
                {5.0f, -2.0f, 7.0f, 1.0f}, "Scaling preserves translation");

    MatrixTransform::loadIdentity(matrix.data());
    MatrixTransform::rotateZ(matrix.data(), 90.0f);
    MatrixTransform::translate(matrix.data(), 5.0f, -2.0f, 7.0f);
    MatrixTransform::scale(matrix.data(), 2.0f);
    expectPoint(applyPoint(matrix.data(), {1.0f, 1.0f, 1.0f, 1.0f}),
                {3.0f, 0.0f, 9.0f, 1.0f}, "Uniform scale on a rotated matrix");

    MatrixTransform::loadIdentity(matrix.data());
    MatrixTransform::scale(matrix.data(), -2.0f, 0.0f, 3.0f);
    expectPoint(applyPoint(matrix.data(), {1.0f, 2.0f, 3.0f, 1.0f}),
                {-2.0f, 0.0f, 9.0f, 1.0f}, "Reflection and zero-axis scale");

    matrix = {0.0f, -1.0f, 0.0f, 5.0f,
              1.0f, 0.0f, 0.0f, -2.0f,
              0.0f, 0.0f, 1.0f, 7.0f,
              0.25f, 0.5f, 0.75f, 1.0f};
    MatrixTransform::scale(matrix.data(), 2.0f, 3.0f, 4.0f);
    expectPoint(applyPoint(matrix.data(), {1.0f, 2.0f, 3.0f, 1.0f}),
                {-1.0f, 0.0f, 19.0f, 13.5f}, "Scale includes a non-affine fourth row");

    int rejected = 0;
    try { MatrixTransform::scale(nullptr, 1.0f); }
    catch (const MatrixTransformException &) { ++rejected; }
    try { MatrixTransform::scale(nullptr, 1.0f, 2.0f, 3.0f); }
    catch (const MatrixTransformException &) { ++rejected; }
    try { MatrixTransform::perspective(nullptr, 45.0f, 1.0f, 0.1f, 100.0f); }
    catch (const MatrixTransformException &) { ++rejected; }
    require(rejected == 3, "Null matrix pointers must throw");
}

void testCameraAndDefaultModel() {
    Camera camera(800.0f, 600.0f);
    const GLfloat aspect = camera.aspect;
    for (const auto &size : {Point{0.0f, 0.0f}, Point{0.0f, 600.0f},
                             Point{800.0f, 0.0f}, Point{-1.0f, 600.0f}}) {
        camera.updateAspect(size[0], size[1]);
        camera.updateProjection();
        require(camera.aspect == aspect, "Empty framebuffer preserves valid aspect");
    }
    camera.updateAspect(640.0f, 640.0f);
    camera.updateProjection();
    require(camera.projectionMatrix[0] == camera.projectionMatrix[5],
            "Valid resize updates projection aspect");

    Transformation model;
    model.updateModelMatrix();
    for (GLfloat x : {-1.0f, 1.0f}) {
        for (GLfloat y : {-1.0f, 1.0f}) {
            for (GLfloat z : {-1.0f, 1.0f}) {
                const Point world = applyPoint(model.modelMatrix, {x, y, z, 1.0f});
                const Point clip = applyPoint(camera.projectionMatrix, world);
                require(clip[3] > 0.0f && std::fabs(clip[0]) <= clip[3]
                        && std::fabs(clip[1]) <= clip[3] && std::fabs(clip[2]) <= clip[3],
                        "Default example cube stays inside the corrected camera frustum");
            }
        }
    }
}

} // namespace

int main() {
    try {
        testPerspective();
        testInvalidProjection();
        testScale();
        testCameraAndDefaultModel();
        std::cout << "Passed " << checks << " projection/scaling assertions.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Matrix test failed: " << error.what() << '\n';
        return 1;
    }
}
