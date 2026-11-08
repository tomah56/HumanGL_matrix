#include "../src/gui/ControlsPanel.hpp"
#include "../src/window/Window.hpp"
#include "../src/render/Render.hpp"
#include "../src/scene/Humanoid.hpp"
#include "../src/animation/Sequence.hpp"
#include "../src/inputHandler/InputData.hpp"
#include "imgui.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}

void saveFrame(const char *path, int width, int height) {
    std::vector<GLubyte> pixels(width * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    std::ofstream file(path, std::ios::binary);
    file << "P6\n" << width << ' ' << height << "\n255\n";
    for (int row = height - 1; row >= 0; --row) {
        file.write(reinterpret_cast<const char *>(pixels.data() + row * width * 3), width * 3);
    }
}
}

int main(int argc, char **argv) {
    try {
        require(glfwInit(), "A display session is required");
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        Window window(1200, 900, "HumanGL controls checks");
        require(gladLoadGL((GLADloadfunc)glfwGetProcAddress), "GLAD initialization failed");
        ShaderProgram shader;
        Render render(shader);
        Humanoid body;
        animation::SequencePlayer player;
        Transformation transformation;
        transformation.animationState.isRotate = false;
        transformation.transform.scaleFactor = 1;
        transformation.transform.translationY = 1.3f;
        transformation.transform.translationZ = -11;
        transformation.transform.rotationY = 25;
        Camera camera(1200, 900);
        InputData input(&transformation, nullptr, &camera);
        input.body = &body;
        input.player = &player;
        window.setUpUserInput(input);
        {
            ControlsPanel panel(window.nativeHandle());
            int width, height;
            window.getFramebufferSize(width, height);
            auto frame = [&] {
                panel.beginFrame();
                panel.draw(input);
                const int sceneWidth = panel.sceneFramebufferWidth();
                require(sceneWidth > 0 && sceneWidth < width, "Panel reserves space beside the 3D view");
                glViewport(0, 0, sceneWidth, height);
                camera.updateAspect(sceneWidth, height);
                camera.updateProjection();
                transformation.updateModelMatrix();
                render.beginFrame(camera);
                MatrixStack root;
                root.load(transformation.modelMatrix);
                const auto models = body.modelMatrices(root);
                for (std::size_t part = 0; part < models.size(); ++part) {
                    MatrixStack segment;
                    segment.load(models[part]);
                    render.drawUnitCube(segment, part == 0 ? std::array<GLfloat, 3>{0.2f, 0.8f, 0.4f}
                        : part == 9 ? std::array<GLfloat, 3>{0.95f, 0.8f, 0.55f}
                        : part % 2 ? std::array<GLfloat, 3>{1.0f, 0.6f, 0.2f}
                                   : std::array<GLfloat, 3>{0.25f, 0.55f, 1.0f});
                }
                panel.render();
                require(glGetError() == GL_NO_ERROR, "Panel and scene must render without OpenGL errors");
            };
            frame();
            frame(); // Let first-frame layout settle, including the scrollbar.
            require(ImGui::GetDrawData()->TotalVtxCount > 1000, "Panel renders text and controls");
            if (argc > 1) saveFrame(argv[1], width, height);

            auto &io = ImGui::GetIO();
            io.AddFocusEvent(true);
            io.AddMousePosEvent(840, 127);
            frame();
            io.AddMouseButtonEvent(0, true);
            frame();
            io.AddMouseButtonEvent(0, false);
            frame();
            require(player.isPlaying(), "Clicking Walk starts playback through the shared command handler");
            player.update(0.4, body);
            require(body.jointAngle(Joint::LeftHip) == 25, "Mouse-started walk reaches its first pose");
            InputHandler::getInstance().pressKey(input, GLFW_KEY_B);
            require(!player.isPlaying() && body.jointAngle(Joint::LeftHip) == 0,
                    "Keyboard and panel use the same stop action");

            InputHandler::getInstance().pressKey(input, GLFW_KEY_C);
            io.AddMousePosEvent(1065, 453);
            frame();
            io.AddMouseButtonEvent(0, true);
            frame();
            io.AddMouseButtonEvent(0, false);
            frame();
            require(!player.isPlaying() && body.jointAngle(Joint::LeftElbow) == 5,
                    "Clicking a joint button moves that joint and interrupts playback");
            require(body.jointAngle(Joint::LeftShoulder) == 0 && body.jointAngle(Joint::RightElbow) == 0,
                    "A forearm button preserves the upper arm and the other side");
            require(io.WantCaptureMouse, "The panel captures mouse input over its buttons");
            transformation.dragState.isDragging = false;
            InputHandler::mouse_button_callback(window.nativeHandle(), GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
            require(!transformation.dragState.isDragging, "Panel clicks must not drag the scene");
            const GLfloat z = transformation.transform.translationZ;
            InputHandler::scroll_callback(window.nativeHandle(), 0, 1);
            require(transformation.transform.translationZ == z, "Panel scrolling must not zoom the scene");
            io.WantCaptureKeyboard = true;
            InputHandler::key_callback(window.nativeHandle(), GLFW_KEY_C, 0, GLFW_PRESS, 0);
            require(!player.isPlaying(), "Captured keyboard input must not trigger scene actions");
            io.WantCaptureKeyboard = false;
            InputHandler::key_callback(window.nativeHandle(), GLFW_KEY_M, 0, GLFW_PRESS, 0);
            require(player.isPlaying(), "Existing keyboard commands work alongside the panel");
        }
        require(ImGui::GetCurrentContext() == nullptr, "Panel destroys its UI context");
        render.cleanUp();
        shader.deleteShaderProgram();
        require(glGetError() == GL_NO_ERROR, "Panel cleanup must preserve the OpenGL context");
        std::cout << "Passed controls-panel rendering, mouse playback, shared commands, input capture, and cleanup checks.\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << "Controls panel test failed: " << error.what() << '\n';
        return 1;
    }
}
