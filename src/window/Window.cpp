#include "Window.hpp"
#include "../inputHandler/InputData.hpp"
#include "../inputHandler/InputHandler.hpp"
#include <string>
#include <iostream>
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace std;

WindowException::WindowException(ErrorCode err)
	: _errorCode(err) {}

const char *WindowException::what() const noexcept {
	switch (_errorCode) {
		case INVALID_RESOLUTION: return "Invalid screen resolution";
		case CANNOT_INITIALIZE_GLFW: return "Cannot initialize GLFW";
		case CANNOT_CREATE_WINDOW: return "GLFW cannot create a window";
		default: return "An unknown error occured during shader creating";
	}
}


/**
* @brief Create a GLFW window with specified width, height, and title.
*
* Initializes GLFW, checks screen resolution, sets OpenGL hints,
* creates a window, and makes its context current.
*
* If width or height <= 0, defaults to half the screen resolution.
* If name is null or empty, defaults to "Hello triangle".
*
* @param width Desired window width (pixels).
* @param height Desired window height (pixels).
* @param name Window title.
* @throws WindowException on GLFW init failure, invalid resolution, or window creation failure.
*/
Window::Window(const int width, const int height, const char *name)
	: _windowName(name)
{
	// Initialize the library
	if (!glfwInit()) {
		throw WindowException(WindowException::CANNOT_INITIALIZE_GLFW);
	}

	if (_windowName == nullptr || _windowName[0] == '\0') {
		_windowName = "〔^▴^〕(^.^)〈°⥿°〉";
	}

	getResolution();

	_width = (width <= 0) ? _maxWidth : width;
	_height = (height <= 0) ? _maxHeight : height;

	if (_width <= 0 || _height <= 0) {
		throw WindowException(WindowException::INVALID_RESOLUTION);
	}

	// Use version 4.1 (last available on MacOS)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	cout << "Creating a window..." << endl;

	// Create a GLFWindow object and its OpenGL context
	_window = glfwCreateWindow(_width, _height, _windowName, NULL, NULL);
	if (!_window) {
		glfwTerminate();
		throw WindowException(WindowException::CANNOT_CREATE_WINDOW);
	}

	// Make the window's context current
	glfwMakeContextCurrent(_window);
}

// getters //

int Window::getScreenWidth() const {
	return _width;
}

int Window::getScreenHeight() const {
	return _height;
}

// other //

void Window::setUpUserInput(InputData &inputData) {
	// Set user pointer for callback access
	glfwSetWindowUserPointer(_window, &inputData);

	// Set input callback
	glfwSetKeyCallback(_window, InputHandler::key_callback);
	glfwSetMouseButtonCallback(_window, InputHandler::mouse_button_callback);
	glfwSetScrollCallback(_window, InputHandler::scroll_callback);
	glfwSetCursorPosCallback(_window, InputHandler::cursor_position_callback);
	glfwSetFramebufferSizeCallback(_window, InputHandler::framebuffer_size_callback);
;}

/**
* @brief Check if the window received a close request.
* @return true if the window should close, false otherwise.
*/
bool Window::windowShouldClose() const {
	return glfwWindowShouldClose(_window);
}

/**
* @brief Swap the front and back buffers for double-buffered rendering.
*/
void Window::swapBuffers() const {
	// Swap front and back buffers
	glfwSwapBuffers(_window);
}

/**
* @brief Destroy the window and terminate GLFW.
*
* Automatically called when the Window object goes out of scope.
*/
Window::~Window() {
	if (_window) {
		glfwDestroyWindow(_window);
	}
	glfwTerminate();
	cout << "Window was terminated" << endl;
}

// private //

/**
* @brief Detect the current screen resolution of the primary monitor.
*
* Sets _maxWidth and _maxHeight based on GLFW video mode.
* Throws WindowException if resolution cannot be determined.
*/
void Window::getResolution() {
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	_maxWidth = mode->width;
	_maxHeight = mode->height;
}
