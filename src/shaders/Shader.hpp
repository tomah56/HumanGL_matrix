#pragma once

#include <string>
#include <exception>
#include <glad/gl.h>

class ShaderException : public std::exception {
public:
	enum ErrorCode {
		TYPE_NOT_FOUND,
		SOURCE_NOT_FOUND,
		COMPILATION_FAILED,
		UNKNOWN_ERROR,
	};

	explicit ShaderException(ErrorCode err);
	const char *what() const noexcept override;

private:
	ErrorCode _errorCode;
};

class Shader {
public:
	explicit Shader(const char *fileName, GLenum shaderType);

	GLuint getShader() const;
	void deleteShader();

private:
	std::string _shaderSourceStr;
	const char *_shaderSource;
	GLenum _shaderType;
	GLuint _shader;
	static const GLuint _shaderTypes[5];

	bool isValidShaderType(GLenum type) const;
	std::string getFileContent(const char *fileName);
	Shader();
};
