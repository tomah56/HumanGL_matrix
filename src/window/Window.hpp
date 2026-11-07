
/**
* @file Window.hpp
* @brief Encapsulates a GLFW window with basic setup and utility functions.
*
* This module wraps GLFW window creation and management into a
* convenient C++ class. It handles initialization, resolution
* detection, and basic window operations like buffer swapping
* and close checks.
*
* Throws WindowException for critical failures during setup.
*/

#pragma once

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <exception>

struct InputData;

/**
* @class WindowException
* @brief Exception type used for window creation and setup errors.
*
* Provides categorized error codes for various failures
* that can occur during GLFW initialization and window creation.
*/
class WindowException : public std::exception {
public:
	enum ErrorCode {
		INVALID_RESOLUTION,
		CANNOT_INITIALIZE_GLFW,
		CANNOT_CREATE_WINDOW,
	};

	explicit WindowException(ErrorCode err);
	const char *what() const noexcept override;

private:
	ErrorCode _errorCode;
};


/**
* @class Window
* @brief Wraps a GLFW window and provides utility functions.
*
* This class encapsulates GLFW window creation, resolution detection,
* and basic window operations such as checking if the window should
* close and swapping the buffers.
*
* Usage example:
* @code
* Window win(1280, 720, "My OpenGL Window");
* while (!win.windowShouldClose()) {
*     // Rendering code
*     win.swapBuffers();
* }
* @endcode
*
* Throws WindowException if GLFW fails to initialize, the window cannot
* be created, or the screen resolution is invalid.
*/
class Window {
public:
	Window(const int width, const int height, const char *name);

	int getScreenWidth() const;
	int getScreenHeight() const;

	void setUpUserInput(InputData &inputData);
	bool windowShouldClose() const;
	void swapBuffers() const;

	~Window();

private:
	GLFWwindow* _window = nullptr;
	int _maxWidth = 0, _maxHeight = 0;
	int _width = 0, _height = 0;
	const char * _windowName = nullptr;

	void getResolution();
	Window();
};
