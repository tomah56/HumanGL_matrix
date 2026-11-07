#pragma once

#include <exception>
#include <glad/gl.h>

class MatrixTransformException : public std::exception {
public:
	enum ErrorCode {
		INVALID_MATRIX,
	};

	explicit MatrixTransformException(ErrorCode err);
	const char *what() const noexcept override;

private:
	ErrorCode _errorCode;
};

class MatrixTransform {
public:
	static void scale(GLfloat *matrix, GLfloat scaleFactor);
	static void rotateX(GLfloat *matrix, GLfloat angleDegree);
	static void rotateY(GLfloat *matrix, GLfloat angleDegree);
	static void rotateZ(GLfloat *matrix, GLfloat angleDegree);
	static void translate(GLfloat *matrix, GLfloat x, GLfloat y, GLfloat z);
	static void perspective(GLfloat *matrix, GLfloat fov, GLfloat aspect, GLfloat near, GLfloat far);

	static void loadIdentity(GLfloat *matrix);
	static void multiply(GLfloat *m1, GLfloat *m2, GLfloat *result);
	static GLfloat degreesToRadians(GLfloat degrees);
};
