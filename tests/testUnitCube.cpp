#include "../src/render/UnitCube.hpp"

#include <array>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>

int main() {
    try {
        const Mesh cube = makeUnitCubeMesh();
        if (cube.vertices.size() != 8 * 11 || cube.indices.size() != 36) {
            throw std::runtime_error("Cube must have eight interleaved corners and 36 indices");
        }
        using Point = std::array<GLfloat, 3>;
        std::set<Point> corners;
        for (std::size_t i = 0; i < cube.vertices.size(); i += 11) {
            Point point = {cube.vertices[i], cube.vertices[i + 1], cube.vertices[i + 2]};
            for (GLfloat coordinate : point) {
                if (std::fabs(coordinate) != 0.5f) throw std::runtime_error("Cube is not centered unit geometry");
            }
            corners.insert(point);
        }
        if (corners.size() != 8) throw std::runtime_error("Cube corners must be distinct");

        std::map<std::pair<GLuint, GLuint>, int> edges;
        std::set<Point> normals;
        GLfloat area = 0.0f;
        for (std::size_t i = 0; i < cube.indices.size(); i += 3) {
            Point p[3];
            for (int j = 0; j < 3; ++j) {
                const GLuint index = cube.indices[i + j];
                if (index >= 8) throw std::runtime_error("Cube index is out of bounds");
                for (int axis = 0; axis < 3; ++axis) p[j][axis] = cube.vertices[index * 11 + axis];
                const GLuint next = cube.indices[i + (j + 1) % 3];
                ++edges[std::minmax(index, next)];
            }
            Point a, b;
            for (int axis = 0; axis < 3; ++axis) {
                a[axis] = p[1][axis] - p[0][axis];
                b[axis] = p[2][axis] - p[0][axis];
            }
            const Point cross = {a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]};
            GLfloat outward = 0.0f;
            GLfloat lengthSquared = 0.0f;
            for (int axis = 0; axis < 3; ++axis) {
                outward += cross[axis] * (p[0][axis] + p[1][axis] + p[2][axis]) / 3.0f;
                lengthSquared += cross[axis] * cross[axis];
            }
            if (outward <= 0.0f) throw std::runtime_error("Triangle winding is not outward");
            area += 0.5f * std::sqrt(lengthSquared);
            normals.insert(cross);
        }
        for (const auto &[edge, uses] : edges) {
            (void)edge;
            if (uses != 2) throw std::runtime_error("Cube surface has a boundary or duplicate triangle");
        }
        if (normals.size() != 6 || area != 6.0f) throw std::runtime_error("Cube must have six unit faces");
        std::cout << "Passed unit-cube extent, topology, winding, and surface-area checks.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Unit-cube test failed: " << error.what() << '\n';
        return 1;
    }
}
