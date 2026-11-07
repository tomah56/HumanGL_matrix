#include "MatrixTransform.hpp"
#include <glad/gl.h>
#include <string.h>
#include <cmath>

using namespace std;

MatrixTransformException::MatrixTransformException(ErrorCode err)
	: _errorCode(err) {}

const char *MatrixTransformException::what() const noexcept {
	switch (_errorCode) {
		case INVALID_MATRIX: return "Invalid matrix";
		case INVALID_PROJECTION: return "Invalid perspective parameters";
		default: return "An unknown error occured during matrix transformation";
	}
}


// scale
void MatrixTransform::scale(GLfloat *matrix, GLfloat scaleFactor) {
    scale(matrix, scaleFactor, scaleFactor, scaleFactor);
}

void MatrixTransform::scale(GLfloat *matrix, GLfloat x, GLfloat y, GLfloat z) {
    if (!matrix) throw MatrixTransformException(MatrixTransformException::INVALID_MATRIX);

    // M * S scales the first three columns, including their off-diagonal entries.
    for (int row = 0; row < 4; ++row) {
        matrix[row * 4] *= x;
        matrix[row * 4 + 1] *= y;
        matrix[row * 4 + 2] *= z;
    }
}


// rotate
void MatrixTransform::rotateX(GLfloat *matrix, GLfloat angleDegree) {
    if (!matrix) throw MatrixTransformException(MatrixTransformException::INVALID_MATRIX);

    GLfloat c = cosf(degreesToRadians(angleDegree));
    GLfloat s = sinf(degreesToRadians(angleDegree));

    matrix[5] = c;
    matrix[6] = -s;
    matrix[9] = s;
    matrix[10] = c;
}

void MatrixTransform::rotateY(GLfloat *matrix, GLfloat angleDegree) {
    if (!matrix) throw MatrixTransformException(MatrixTransformException::INVALID_MATRIX);

    GLfloat c = cosf(degreesToRadians(angleDegree));
    GLfloat s = sinf(degreesToRadians(angleDegree));

    matrix[0] = c;
    matrix[2] = -s;
    matrix[8] = s;
    matrix[10] = c;
}

void MatrixTransform::rotateZ(GLfloat *matrix, GLfloat angleDegree) {
    if (!matrix) throw MatrixTransformException(MatrixTransformException::INVALID_MATRIX);

    GLfloat c = cosf(degreesToRadians(angleDegree));
    GLfloat s = sinf(degreesToRadians(angleDegree));

    matrix[0] = c;
    matrix[1] = -s;
    matrix[4] = s;
    matrix[5] = c;
}


// translate
void MatrixTransform::translate(GLfloat *matrix, GLfloat x, GLfloat y, GLfloat z) {
    if (!matrix) throw MatrixTransformException(MatrixTransformException::INVALID_MATRIX);

	matrix[3] = matrix[3] + x;
    matrix[7] = matrix[7] + y;
    matrix[11] = matrix[11] + z;
}


// identity matrix
void MatrixTransform::loadIdentity(GLfloat *matrix) {
	for (int i = 0; i < 16; ++i)
        matrix[i] = 0.0f;

	matrix[0] = 1.0f;
	matrix[5] = 1.0f;
	matrix[10] = 1.0f;
	matrix[15] = 1.0f;
}


// matrix multiplication
void MatrixTransform::multiply(const GLfloat *m1, const GLfloat *m2, GLfloat *result) {
    if (!m1 || !m2 || !result) throw MatrixTransformException(MatrixTransformException::INVALID_MATRIX);

    GLfloat temp[16];
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            GLfloat sum = 0;
            for (int k = 0; k < 4; ++k) {
                sum += m1[row * 4 + k] * m2[k * 4 + col];
            }
            temp[row * 4 + col] = sum;
        }
    }
    memcpy(result, temp, sizeof(GLfloat) * 16);
}


// other
void MatrixTransform::perspective(GLfloat *matrix, GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far) {
    if (!matrix) throw MatrixTransformException(MatrixTransformException::INVALID_MATRIX);
    if (!std::isfinite(fov) || !std::isfinite(aspect) || !std::isfinite(near)
        || !std::isfinite(far) || fov <= 0.0f || fov >= 180.0f || aspect <= 0.0f
        || near <= 0.0f || far <= near) {
        throw MatrixTransformException(MatrixTransformException::INVALID_PROJECTION);
    }

    GLfloat f = 1.0f / std::tan(degreesToRadians(fov) / 2.0f);
    GLfloat projection[16] = {};

    projection[0] = f / aspect;
    projection[5] = f;
    projection[10] = (far + near) / (near - far);
    projection[11] = (2.0f * far * near) / (near - far);
    projection[14] = -1.0f;
    memcpy(matrix, projection, sizeof(projection));
}

GLfloat MatrixTransform::degreesToRadians(GLfloat degrees) {
    return degrees * (3.14159265f / 180.0f);
}
