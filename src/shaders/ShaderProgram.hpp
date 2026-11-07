#pragma once

#include <vector>
#include <exception>
#include <glad/gl.h>
#include "Shader.hpp"

class ShaderProgramException : public std::exception {
public:
	enum ErrorCode {
		LINKING_FAILED,
	};

	explicit ShaderProgramException(ErrorCode err);
	const char *what() const noexcept override;

private:
	ErrorCode _errorCode;
};

class ShaderProgram {
public:
	ShaderProgram();
	GLuint getShaderProgram() const;
	void deleteShaderProgram();

private:
	GLuint _shaderProgram;
};
