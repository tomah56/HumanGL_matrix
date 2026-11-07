#include "Camera.hpp"
#include "../matrixMath/MatrixTransform.hpp"

Camera::Camera(GLfloat width, GLfloat height)
	: width(width), height(height)
{
	updateAspect(width, height);
	updateView();
	updateProjection();
}

void Camera::updateProjection() {
	MatrixTransform::perspective(projectionMatrix, fov, aspect, near, far);
}

void Camera::updateView() {
	MatrixTransform::loadIdentity(viewMatrix);
	MatrixTransform::translate(viewMatrix, 0.0f, 0.0f, 0.0f);
}

void Camera::updateAspect(GLfloat width, GLfloat height) {
	// Preserve the last valid projection while a framebuffer is minimized.
	if (width <= 0.0f || height <= 0.0f) return;

	aspect = width / height;
}
