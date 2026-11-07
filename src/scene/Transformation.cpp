#include "Transformation.hpp"
#include "../matrixMath/MatrixTransform.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>

void Transformation::updateModelMatrix() {
    MatrixTransform::loadIdentity(modelMatrix);
    MatrixTransform::translate(modelMatrix, transform.translationX, transform.translationY, transform.translationZ);

    GLfloat totalRotationY = transform.rotationY;
    if (animationState.isRotate) {
        totalRotationY = transform.baseRotationY + animationState.rotation;
    }

    MatrixTransform::loadIdentity(rotateY);
    MatrixTransform::rotateY(rotateY, totalRotationY);

    MatrixTransform::loadIdentity(rotateX);
    MatrixTransform::rotateX(rotateX, transform.rotationX);

    MatrixTransform::loadIdentity(rotateZ);
    MatrixTransform::rotateZ(rotateZ, transform.rotationZ);

    GLfloat rotYX[16];
    MatrixTransform::multiply(rotateY, rotateX, rotYX);
    GLfloat rotYXZ[16];
    MatrixTransform::multiply(rotYX, rotateZ, rotYXZ);

    GLfloat tempModel[16];
    MatrixTransform::multiply(modelMatrix, rotYXZ, tempModel);

    MatrixTransform::loadIdentity(scale);
    MatrixTransform::scale(scale, transform.scaleFactor);
    MatrixTransform::multiply(tempModel, scale, modelMatrix);
}

// void Transformation::updateModelMatrix() {

// 	MatrixTransform::loadIdentity(scale);
// 	MatrixTransform::scale(scale, transform.scaleFactor);

// 	MatrixTransform::loadIdentity(rotateY);
// 	MatrixTransform::rotateY(rotateY, transform.rotationY);

// 	MatrixTransform::loadIdentity(rotateX);
// 	MatrixTransform::rotateX(rotateX, transform.rotationX);

// 	MatrixTransform::loadIdentity(rotateZ);
// 	MatrixTransform::rotateZ(rotateZ, transform.rotationZ);

// 	MatrixTransform::multiply(rotateY, rotateX, temp);
// 	MatrixTransform::multiply(temp, rotateZ, temp);
// 	MatrixTransform::multiply(temp, scale, modelMatrix);

// 	MatrixTransform::translate(modelMatrix, transform.translationX, transform.translationY, transform.translationZ);

//     if (animationState.isRotate) {
//         GLfloat autoRotateY[16];
//         MatrixTransform::loadIdentity(autoRotateY);
//         MatrixTransform::rotateY(autoRotateY, animationState.rotation);

//         GLfloat result[16];
//         MatrixTransform::multiply(autoRotateY, modelMatrix, result);
//         for (int i = 0; i < 16; i++) {
//             modelMatrix[i] = result[i];
//         }
//     }
// }

