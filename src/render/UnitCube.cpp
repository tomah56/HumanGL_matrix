#include "UnitCube.hpp"

Mesh makeUnitCubeMesh() {
    static const GLfloat positions[8][3] = {
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
        {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
    };
    Mesh mesh;
    mesh.vertices.reserve(8 * 11);
    for (const auto &position : positions) {
        // Retain scop's position/UV/vertex-color/face-color layout.
        mesh.vertices.insert(mesh.vertices.end(), {
            position[0], position[1], position[2],
            position[0] + 0.5f, position[1] + 0.5f,
            1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        });
    }
    mesh.indices = {
        0, 3, 2, 0, 2, 1, // -Z
        4, 5, 6, 4, 6, 7, // +Z
        0, 4, 7, 0, 7, 3, // -X
        1, 2, 6, 1, 6, 5, // +X
        0, 1, 5, 0, 5, 4, // -Y
        3, 7, 6, 3, 6, 2, // +Y
    };
    return mesh;
}
