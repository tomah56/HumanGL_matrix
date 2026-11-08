#include <iostream>
#include <exception>
#include <vector>
#include <memory>
#include <string>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "modelLoader/RenderModelLoader.hpp"
#include "window/Window.hpp"
#include "shaders/ShaderProgram.hpp"
#include "texture/Texture.hpp"
#include "render/Render.hpp"
#include "scene/Transformation.hpp"
#include "scene/Material.hpp"
#include "scene/Camera.hpp"
#include "scene/Humanoid.hpp"
#include "animation/Sequence.hpp"
#include "inputHandler/InputData.hpp"

using namespace std;

static void updateViewer(double deltaTime, Transformation &transformation, Material &material) {
    if (transformation.animationState.isRotate) {
        transformation.animationState.rotation += 15.0f * deltaTime;
    }
    auto &blend = material.materialBlend;
    if (blend.mixValue < blend.targetMixValue) {
        blend.mixValue += blend.transitionSpeed * deltaTime;
        if (blend.mixValue > blend.targetMixValue) blend.mixValue = blend.targetMixValue;
    } else if (blend.mixValue > blend.targetMixValue) {
        blend.mixValue -= blend.transitionSpeed * deltaTime;
        if (blend.mixValue < blend.targetMixValue) blend.mixValue = blend.targetMixValue;
    }
}

static void drawCubeDemo(const Render &render, const Transformation &transformation) {
    MatrixStack stack;
    stack.load(transformation.modelMatrix);

    if (!stack.push()) throw runtime_error("Matrix stack overflow");
    stack.scale(0.8f);
    render.drawUnitCube(stack, {1.0f, 0.6f, 0.2f});
    if (!stack.pop()) throw runtime_error("Matrix stack underflow");

    // Each sibling starts from the same unscaled parent frame.
    for (int side : {-1, 1}) {
        if (!stack.push()) throw runtime_error("Matrix stack overflow");
        stack.translate(side * 1.4f, 0.0f, 0.0f);
        if (side < 0) stack.rotateZ(25.0f);
        stack.scale(0.6f, side < 0 ? 1.0f : 0.6f, 0.6f);
        render.drawUnitCube(stack, side < 0 ? array<GLfloat, 3>{0.2f, 0.8f, 0.4f}
                                            : array<GLfloat, 3>{0.3f, 0.5f, 1.0f});
        if (!stack.pop()) throw runtime_error("Matrix stack underflow");
    }
}

static void drawBody(const Render &render, const Humanoid &body,
                     const Transformation &transformation, GLfloat height) {
    MatrixStack stack;
    stack.translate(0.0f, height, 0.0f);
    stack.multiply(transformation.modelMatrix);
    const auto models = body.modelMatrices(stack);
    const array<array<GLfloat, 3>, 2> colors = {{{1.0f, 0.6f, 0.2f}, {0.25f, 0.55f, 1.0f}}};
    for (size_t part = 0; part < models.size(); ++part) {
        stack.load(models[part]);
        render.drawUnitCube(stack, part == 0 ? array<GLfloat, 3>{0.2f, 0.8f, 0.4f}
                                 : part == 9 ? array<GLfloat, 3>{0.95f, 0.8f, 0.55f}
                                           : colors[(part - 1) % colors.size()]);
    }
}

int main(int args, char* argv[]) {

    int width = 0, height = 0;
    char *windowName = nullptr;

    const bool cubeDemo = args == 2 && string(argv[1]) == "--cubes";
    const bool limbDemo = args == 2 && (string(argv[1]) == "--limbs"
                                      || string(argv[1]) == "--arms" || string(argv[1]) == "--arm");
    const bool primitiveDemo = cubeDemo || limbDemo;
    if (!primitiveDemo && args != 3) {
        cout << "Usage: ./humangl --limbs (or --arms / --arm)\n"
             << "       ./humangl --cubes\n"
             << "       ./humangl models/cube.obj textureSources/wood.bmp" << endl;
        return args == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    try {
        Mesh mesh;
        if (!primitiveDemo) {
            RenderModelLoader renderModel(argv[1]);
            mesh = renderModel.getMesh();
        }

        Window window(width, height, limbDemo ? "HumanGL | Articulated body" : windowName);

        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
            throw runtime_error("Failed to initialize GLAD");
        }

        // // Check OpenGL version
        // const GLubyte* version = glGetString(GL_VERSION);
        // cout << "OpenGL version: " << version << endl;

        ShaderProgram shaderProgram;
        Render render(mesh, shaderProgram);
        unique_ptr<Texture> texture;
        if (!primitiveDemo) texture = make_unique<Texture>(argv[2]);

        Humanoid body;
        animation::SequencePlayer player;
        Transformation transformation;
        if (primitiveDemo) {
            transformation.animationState.isRotate = false;
            transformation.transform.scaleFactor = 1.0f;
            transformation.transform.translationZ = -6.0f;
        }
        if (limbDemo) {
            transformation.transform.translationX = 0.0f;
            transformation.transform.translationY = (body.limbDimensions.upper.length
                + body.limbDimensions.forearm.length) / 2.0f;
            transformation.transform.translationZ = -11.0f; // Leave room above the head for jumping.
            transformation.minZ = -20.0f;
            transformation.transform.rotationY = 25.0f;
        }
        Material material;
        Camera camera((GLfloat)window.getScreenWidth(), (GLfloat)window.getScreenHeight());
        InputData inputData = {&transformation, &material, &camera};
        if (limbDemo) {
            inputData.body = &body;
            inputData.player = &player;
        }

        // Set up user input
        window.setUpUserInput(inputData);
        if (limbDemo) {
            cout << "Body: Left/Right rotate (Shift faster). "
                    "C walk, V jump, B stand still. "
                    "Hold L/R for left/right; L+K or R+T for lower segments. "
                    "A/D arm forward/back, Q/E leg forward/back. "
                    "1/2 all upper lengths, 3/4 all lower lengths, "
                    "0 reset all four limbs. Escape closes.\n";
        }

        // GLFW must be initialized before reading its timer.
        double prevTime = glfwGetTime();

        // Loop until the user closes the window
        while (!window.windowShouldClose()) {
            glfwPollEvents();
            if (window.windowShouldClose()) break;

            window.getFramebufferSize(width, height);
            if (width <= 0 || height <= 0) {
                glfwWaitEvents();
                prevTime = glfwGetTime();
                continue;
            }
            glViewport(0, 0, width, height);
            camera.updateAspect(static_cast<GLfloat>(width), static_cast<GLfloat>(height));

            double currentTime = glfwGetTime();
            double deltaTime = currentTime - prevTime;
            prevTime = currentTime;

            updateViewer(deltaTime, transformation, material);
            if (limbDemo) player.update(deltaTime, body);
            camera.updateView();
            camera.updateProjection();
            transformation.updateModelMatrix();
            render.beginFrame(camera);
            if (limbDemo) drawBody(render, body, transformation, player.heightOffset());
            else if (cubeDemo) drawCubeDemo(render, transformation);
            else render.drawMesh(transformation.modelMatrix, material);

            // Swap front and back buffers
            window.swapBuffers();

        }

        if (texture) texture->unbindTexture();
        render.cleanUp();
        shaderProgram.deleteShaderProgram();
        return 0;

    } catch (const RenderModelLoaderException &e) {
        cerr << "Rendering model error: " << e.what() << endl;
    } catch (const WindowException &e) {
        cerr << "Window error: " << e.what() << endl;
    } catch (const ShaderException &e) {
        cerr << "Shader error: " << e.what() << endl;
    } catch (const ShaderProgramException &e) {
        cerr << "Shader program error: " << e.what() << endl;
    } catch (const MatrixTransformException &e) {
        cerr << "Matrix transformation error: " << e.what() << endl;
    } catch (const BMPLoaderException &e) {
        cerr << "BMP loader error: " << e.what() << endl;
    } catch (const exception &e) {
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "An unknown exception occured" << endl;
    }
    return EXIT_FAILURE;
}
