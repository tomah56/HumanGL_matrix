#include "../src/window/Window.hpp"
#include "../src/render/Render.hpp"
#include "../src/render/UnitCube.hpp"
#include "../src/scene/Humanoid.hpp"
#include "../src/animation/Motions.hpp"

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

        Humanoid body;
        body.pose = {{0, 60}, {0, 60}, {0, 60}, {0, 60}};
        stack.reset();
        stack.translate(0.0f, 1.3f, -9.0f);
        const auto limbRoot = stack.top();
        const auto bodyModels = body.modelMatrices(stack);
        const std::array<std::array<GLfloat, 3>, 2> colors = {{{1.0f, 0.6f, 0.2f}, {0.25f, 0.55f, 1.0f}}};
        render.beginFrame(camera);
        for (std::size_t part = 0; part < bodyModels.size(); ++part) {
            MatrixStack segment;
            segment.load(bodyModels[part]);
            render.drawUnitCube(segment, part == 0 ? std::array<GLfloat, 3>{0.2f, 0.8f, 0.4f}
                                       : part == 9 ? std::array<GLfloat, 3>{0.95f, 0.8f, 0.55f}
                                                  : colors[(part - 1) % colors.size()]);
        }
        expectPixel(320, 410, {242, 204, 140});
        expectPixel(320, 320, {51, 204, 102});
        expectPixel(268, 336, {255, 153, 51});
        expectPixel(372, 336, {255, 153, 51});
        expectPixel(265, 274, {64, 140, 255});
        expectPixel(375, 274, {64, 140, 255});
        expectPixel(300, 221, {255, 153, 51});
        expectPixel(340, 221, {255, 153, 51});
        expectPixel(300, 151, {64, 140, 255});
        expectPixel(340, 151, {64, 140, 255});
        expectPixel(320, 221, {0, 0, 0});
        expectPixel(320, 440, {0, 0, 0});
        require(stack.top() == limbRoot && stack.size() == 1, "Limb traversal preserves its parent frame");
        require(draws == 13 && clears == 3 && uploads == 0 && indexedCubeDraws,
                "Complete body must use exactly ten shared-cube draws without new uploads");
        require(glGetError() == GL_NO_ERROR, "Limb rendering raised an OpenGL error");

        animation::SequencePlayer player;
        player.play(animation::jumpingSequence(), body);
        player.update(0.48, body);
        MatrixStack root;
        root.translate(0.0f, 1.3f, -11.0f);
        MatrixStack elevated;
        elevated.translate(0.0f, player.heightOffset(), 0.0f);
        elevated.multiply(root.top());
        const auto jumpModels = body.modelMatrices(elevated);
        render.beginFrame(camera);
        for (std::size_t part = 0; part < jumpModels.size(); ++part) {
            MatrixStack segment;
            segment.load(jumpModels[part]);
            render.drawUnitCube(segment, part == 0 ? std::array<GLfloat, 3>{0.2f, 0.8f, 0.4f}
                                       : part == 9 ? std::array<GLfloat, 3>{0.95f, 0.8f, 0.55f}
                                                  : colors[(part - 1) % colors.size()]);
        }
        expectPixel(320, 438, {242, 204, 140});
        expectPixel(320, 370, {51, 204, 102});
        expectPixel(320, 475, {0, 0, 0});
        require(draws == 23 && clears == 4 && uploads == 0 && indexedCubeDraws,
                "Jumping body keeps ten shared-cube draws without geometry uploads");
        require(glGetError() == GL_NO_ERROR, "Jump rendering raised an OpenGL error");

        render.cleanUp();
        render.cleanUp();
        require(glIsProgram(shader.getShaderProgram()), "Renderer must not delete a borrowed shader program");
        shader.deleteShaderProgram();
        require(glGetError() == GL_NO_ERROR, "Resource cleanup raised an OpenGL error");
        std::cout << "Passed shared-cube pixels, draw counts, VAO reuse, stack preservation, "
                     "legacy mesh mode, complete body, jump apex, and cleanup checks.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Render test failed: " << error.what() << '\n';
        return 1;
    }
}
