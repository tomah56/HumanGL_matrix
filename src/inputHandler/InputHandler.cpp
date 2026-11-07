#include "InputHandler.hpp"
#include "InputData.hpp"
#include <iostream>
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

InputHandler::InputHandler() { }

InputHandler &InputHandler::getInstance() {
	static InputHandler instance;
	return instance;
}

void InputHandler::handleKeyInput(InputData* inputData, int key, int scancode, int action, int mods) {
	(void)scancode;

	if (action != GLFW_PRESS) return;

	bool hasCtrlOrCmd = (mods & GLFW_MOD_CONTROL) || (mods & GLFW_MOD_SUPER);
	bool hasShift = (mods & GLFW_MOD_SHIFT);

	GLfloat delta = inputData->transformation->animationState.delta;
	// Calculate movement multiplier based on modifiers
	if (hasShift) {
		delta *= 5.0f;
	}

	if (hasCtrlOrCmd) {
		switch (key) {
			case GLFW_KEY_R:
				// Rotation
				if (!inputData->transformation->animationState.isRotate) {
					// Turning ON: save current manual rotation as base
					inputData->transformation->transform.baseRotationY = inputData->transformation->transform.rotationY;
			        inputData->transformation->animationState.rotation = 0.0f;
			    } else {
			        // Turning OFF: bake the auto-rotation into manual rotation
			        inputData->transformation->transform.rotationY = inputData->transformation->transform.baseRotationY + inputData->transformation->animationState.rotation;
			    }
			    inputData->transformation->animationState.isRotate = !inputData->transformation->animationState.isRotate;
				return;
			case GLFW_KEY_T:
				// Toggle texture/color
				inputData->material->materialBlend.textureMode = !inputData->material->materialBlend.textureMode;
				inputData->material->materialBlend.targetMixValue = inputData->material->materialBlend.textureMode ? 1.0f : 0.0f;
				return;
			case GLFW_KEY_F:
				// Toggle color mods
				inputData->material->materialBlend.faceColorMode = !inputData->material->materialBlend.faceColorMode;
				inputData->material->materialBlend.textureMode = false;
				inputData->material->materialBlend.targetMixValue = 0.0f;
				return;
		}
	}

	// Translation
	switch (key) {
		case GLFW_KEY_LEFT:
			inputData->transformation->transform.translationX -= delta;
			return;
		case GLFW_KEY_RIGHT:
			inputData->transformation->transform.translationX += delta;
			return;
		case GLFW_KEY_DOWN:
			inputData->transformation->transform.translationY -= delta;
			return;
		case GLFW_KEY_UP:
			inputData->transformation->transform.translationY += delta;
			return;
		case GLFW_KEY_S:
			// Move backward
			inputData->transformation->transform.translationZ -= delta;
			return;
		case GLFW_KEY_W:
			// Move forward
			inputData->transformation->transform.translationZ += delta;
			return;
	}
}

void InputHandler::handleMouseMove(InputData* inputData, double xPos, double yPos) {
	if (!inputData->transformation->dragState.isDragging) return;

    double deltaX = xPos - inputData->transformation->dragState.lastMouseX;
    double deltaY = yPos - inputData->transformation->dragState.lastMouseY;

	const float sensitivity = 0.5f;

    // Update rotation angles
    // Horizontal movement rotates around Y axis
    inputData->transformation->transform.rotationY += deltaX * sensitivity;

    // Vertical movement rotates around X axis
    inputData->transformation->transform.rotationX += deltaY * sensitivity;

    // // Optional: clamp X rotation to prevent gimbal lock
  	// const float maxAngle = 89.0f;
    // if (inputData->transformation->transform.rotationX > maxAngle)
    //     inputData->transformation->transform.rotationX = maxAngle;
    // if (inputData->transformation->transform.rotationX < -maxAngle)
    //     inputData->transformation->transform.rotationX = -maxAngle;

	inputData->transformation->dragState.lastMouseX = xPos;
	inputData->transformation->dragState.lastMouseY = yPos;
}


// Callback

void InputHandler::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	InputData* inputData = static_cast<InputData*>(glfwGetWindowUserPointer(window));
	if (!inputData) return;

	// Close by ESC
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	getInstance().handleKeyInput(inputData, key, scancode, action, mods);
}

void InputHandler::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	(void)mods;

	InputData* inputData= static_cast<InputData*>(glfwGetWindowUserPointer(window));
	if (!inputData) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            inputData->transformation->dragState.isDragging = true;
            // Get current mouse position
            glfwGetCursorPos(window,
                           &inputData->transformation->dragState.lastMouseX,
                           &inputData->transformation->dragState.lastMouseY);
        } else if (action == GLFW_RELEASE) {
            inputData->transformation->dragState.isDragging = false;
        }
    }
}

void InputHandler::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	(void)xoffset;

	InputData* inputData = static_cast<InputData*>(glfwGetWindowUserPointer(window));
	if (!inputData) return;

	const float scrollSensitivity = 0.1f;

	GLfloat newZ = inputData->transformation->transform.translationZ + yoffset * scrollSensitivity;
	if (newZ <= inputData->transformation->maxZ && newZ >= inputData->transformation->minZ) {
		inputData->transformation->transform.translationZ = newZ;
	}
}

void InputHandler::cursor_position_callback(GLFWwindow *window, double xPos, double yPos) {
	(void)window;

	InputData* inputData = static_cast<InputData*>(glfwGetWindowUserPointer(window));
	if (!inputData) return;

	getInstance().handleMouseMove(inputData, xPos, yPos);
}

void InputHandler::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	InputData* inputData = static_cast<InputData*>(glfwGetWindowUserPointer(window));
	if (!inputData) return;

	glViewport(0, 0, width, height);

	inputData->camera->updateAspect(width, height);
}
