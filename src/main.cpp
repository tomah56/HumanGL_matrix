#include <iostream>
#include <exception>
#include <vector>
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

int main(int args, char* argv[]) {

    int width = 0, height = 0;
    char *windowName = nullptr;

    if (args < 3) {
        cout << "Usage: ./humangl models/cube.obj textureSources/wood.bmp" << endl;
        return 0;
    }
    try {
        RenderModelLoader renderModel(argv[1]);

        Window window(width, height, windowName);

        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
            throw runtime_error("Failed to initialize GLAD");
        }

        // // Check OpenGL version
        // const GLubyte* version = glGetString(GL_VERSION);
        // cout << "OpenGL version: " << version << endl;

        ShaderProgram shaderProgram;
        Render render(renderModel.getMesh(), shaderProgram);
        Texture texture(argv[2]);

        glUseProgram(shaderProgram.getShaderProgram());
        glUniform1i(render.getUniformLocation().texture, 0);

        Transformation transformation;
        Material material;
        Camera camera((GLfloat)window.getScreenWidth(), (GLfloat)window.getScreenHeight());
        InputData inputData = {&transformation, &material, &camera};

        // Set up user input
        window.setUpUserInput(inputData);

        // GLFW must be initialized before reading its timer.
        double prevTime = glfwGetTime();

        // Loop until the user closes the window
        while (!window.windowShouldClose()) {
            // Render here

            double currentTime = glfwGetTime();
            double deltaTime = currentTime - prevTime;
            prevTime = currentTime;

            camera.updateView();
            camera.updateProjection();
            transformation.updateModelMatrix();
            render.renderFrame(deltaTime, transformation, camera, material);

            // A rendering function
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(renderModel.getMesh().indices.size()), GL_UNSIGNED_INT, 0);

            // Swap front and back buffers
            window.swapBuffers();

            // Poll for and process events
            glfwPollEvents();
        }

        texture.unbindTexture();
        render.cleanUp();
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
