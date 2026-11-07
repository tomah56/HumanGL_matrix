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

int main(int args, char* argv[]) {

    int width = 0, height = 0;
    char *windowName = nullptr;

    const bool cubeDemo = args == 2 && string(argv[1]) == "--cubes";
    if (!cubeDemo && args != 3) {
        cout << "Usage: ./humangl --cubes\n"
             << "       ./humangl models/cube.obj textureSources/wood.bmp" << endl;
        return args == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    try {
        Mesh mesh;
        if (!cubeDemo) {
            RenderModelLoader renderModel(argv[1]);
            mesh = renderModel.getMesh();
        }

        Window window(width, height, windowName);

        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
            throw runtime_error("Failed to initialize GLAD");
        }

        // // Check OpenGL version
        // const GLubyte* version = glGetString(GL_VERSION);
        // cout << "OpenGL version: " << version << endl;

        ShaderProgram shaderProgram;
        Render render(mesh, shaderProgram);
        unique_ptr<Texture> texture;
        if (!cubeDemo) texture = make_unique<Texture>(argv[2]);

        Transformation transformation;
        if (cubeDemo) {
            transformation.animationState.isRotate = false;
            transformation.transform.scaleFactor = 1.0f;
            transformation.transform.translationZ = -6.0f;
        }
        Material material;
        Camera camera((GLfloat)window.getScreenWidth(), (GLfloat)window.getScreenHeight());
        InputData inputData = {&transformation, &material, &camera};

        // Set up user input
        window.setUpUserInput(inputData);

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
            camera.updateView();
            camera.updateProjection();
            transformation.updateModelMatrix();
            render.beginFrame(camera);
            if (cubeDemo) drawCubeDemo(render, transformation);
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
