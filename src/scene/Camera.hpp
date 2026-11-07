#pragma once

#include "../matrixMath/MatrixTransform.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <vector>

class Camera {
public:
    // the field of view
    GLfloat fov = 45.0f;
    GLfloat aspect = 1.78f;
    GLfloat near = 0.1f;
    GLfloat far = 100.0f;

	std::vector<float> position; // camera position
	std::vector<float> orientation; // camera direction
	std::vector<float> up; // up direction

	GLfloat viewMatrix[16];
	GLfloat projectionMatrix[16];

	GLfloat width;
	GLfloat height;

	GLfloat speed = 0.1f;
	GLfloat sensitivity = 100.0f;

	explicit Camera(GLfloat width, GLfloat height);

	void updateProjection();
	void updateView();
	void updateAspect(GLfloat width, GLfloat height);

private:
	Camera();
};
