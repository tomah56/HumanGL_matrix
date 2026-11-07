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
	MatrixTransform::loadIdentity(projectionMatrix);
    MatrixTransform::perspective(projectionMatrix, fov, aspect, near, far);
}

void Camera::updateView() {
	MatrixTransform::loadIdentity(viewMatrix);
	MatrixTransform::translate(viewMatrix, 0.0f, 0.0f, 0.0f);
}

void Camera::updateAspect(GLfloat width, GLfloat height) {
	if (height == 0) height = 1.0f;

	aspect = width / height;
}
