#include "../src/window/Window.hpp"
#include "../src/render/Render.hpp"
#include "../src/render/UnitCube.hpp"
#include "../src/scene/Arm.hpp"

#include <array>
#include <iostream>
#include <stdexcept>

namespace {
PFNGLDRAWELEMENTSPROC realDraw = nullptr;
PFNGLCLEARPROC realClear = nullptr;
PFNGLBUFFERDATAPROC realBufferData = nullptr;
int draws = 0;
int clears = 0;
int uploads = 0;
bool indexedCubeDraws = true;

void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}

void GLAD_API_PTR countDraw(GLenum mode, GLsizei count, GLenum type, const void *indices) {
    ++draws;
    indexedCubeDraws = indexedCubeDraws && mode == GL_TRIANGLES && count == 36
                       && type == GL_UNSIGNED_INT && indices == nullptr;
    realDraw(mode, count, type, indices);
}

void GLAD_API_PTR countClear(GLbitfield mask) {
    ++clears;
    realClear(mask);
}

void GLAD_API_PTR countUpload(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
    ++uploads;
    realBufferData(target, size, data, usage);
}

struct DrawHooks {
    DrawHooks() {
        realDraw = glad_glDrawElements;
        realClear = glad_glClear;
        realBufferData = glad_glBufferData;
        glad_glDrawElements = countDraw;
        glad_glClear = countClear;
        glad_glBufferData = countUpload;
    }
    ~DrawHooks() {
        glad_glDrawElements = realDraw;
        glad_glClear = realClear;
        glad_glBufferData = realBufferData;
    }
};

void expectPixel(int x, int y, const std::array<int, 3> &expected) {
    GLubyte pixel[4] = {};
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    for (int channel = 0; channel < 3; ++channel) {
        const int difference = static_cast<int>(pixel[channel]) - expected[channel];
        require(difference >= -3 && difference <= 3, "Unexpected rendered cube/background color");
    }
}
}

int main() {
    try {
        require(glfwInit(), "GLFW could not initialize; a display session is required");
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        Window window(640, 480, "Shared unit-cube checks");
        require(gladLoadGL((GLADloadfunc)glfwGetProcAddress) && GLAD_GL_VERSION_4_1,
                "OpenGL 4.1 is required");
        std::cout << "OpenGL: " << glGetString(GL_VERSION) << '\n';

        ShaderProgram shader;
        Render render(makeUnitCubeMesh(), shader);
        Camera camera(640.0f, 480.0f);
        glViewport(0, 0, 640, 480);
        MatrixStack stack;
        stack.translate(0.0f, 0.0f, -4.0f);
        const auto parent = stack.top();
        DrawHooks hooks;

        render.beginFrame(camera);
        require(stack.push(), "Push left cube");
        stack.translate(-0.8f, 0.0f, 0.0f);
        const auto left = stack.top();
        render.drawUnitCube(stack, {1.0f, 0.0f, 0.0f});
        GLint sharedVAO = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &sharedVAO);
        require(stack.top() == left && stack.size() == 2, "Drawing must not modify the stack");
        require(stack.pop(), "Restore parent after left cube");

        require(stack.push(), "Push right cube");
        stack.translate(0.8f, 0.0f, 0.0f);
        render.drawUnitCube(stack, {0.0f, 1.0f, 0.0f});
        GLint secondVAO = 0;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &secondVAO);
        require(sharedVAO != 0 && secondVAO == sharedVAO, "Cube draws must reuse one VAO");
        require(stack.pop() && stack.top() == parent && stack.size() == 1,
                "Siblings must restore the parent");
        require(draws == 2 && indexedCubeDraws, "Each cube requires exactly one 36-index draw");
        require(clears == 1, "Only beginFrame may clear the frame");
        require(uploads == 0, "Drawing must not upload geometry again");
        expectPixel(200, 240, {255, 0, 0});
        expectPixel(440, 240, {0, 255, 0});
        expectPixel(320, 240, {0, 0, 0});

        // The legacy mesh path must reset the solid-color mode used by cube draws.
        Material material;
        render.beginFrame(camera);
        render.drawMesh(parent.data(), material);
        expectPixel(320, 240, {255, 255, 255});
        require(draws == 3 && clears == 2 && uploads == 0, "Mesh/cube paths share frame setup safely");
        require(glGetError() == GL_NO_ERROR, "Rendering raised an OpenGL error");

        Arm arm;
        arm.pose = {0.0f, 90.0f};
        stack.reset();
        stack.translate(0.0f, 1.0f, -4.0f);
        const auto shoulder = stack.top();
        const auto armModels = arm.modelMatrices(stack);
        const std::array<std::array<GLfloat, 3>, 2> colors = {{{1.0f, 0.6f, 0.2f}, {0.25f, 0.55f, 1.0f}}};
        render.beginFrame(camera);
        for (std::size_t part = 0; part < armModels.size(); ++part) {
            MatrixStack segment;
            segment.load(armModels[part]);
            render.drawUnitCube(segment, colors[part]);
        }
        expectPixel(320, 320, {255, 153, 51});
        expectPixel(410, 180, {64, 140, 255});
        require(stack.top() == shoulder && stack.size() == 1, "Arm traversal preserves its shoulder frame");
        require(draws == 5 && clears == 3 && uploads == 0 && indexedCubeDraws,
                "Arm must use exactly two shared-cube draws without new uploads");
        require(glGetError() == GL_NO_ERROR, "Arm rendering raised an OpenGL error");

        render.cleanUp();
        render.cleanUp();
        require(glIsProgram(shader.getShaderProgram()), "Renderer must not delete a borrowed shader program");
        shader.deleteShaderProgram();
        require(glGetError() == GL_NO_ERROR, "Resource cleanup raised an OpenGL error");
        std::cout << "Passed shared-cube pixels, draw counts, VAO reuse, stack preservation, "
                     "legacy mesh mode, articulated arm, and cleanup checks.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Render test failed: " << error.what() << '\n';
        return 1;
    }
}
