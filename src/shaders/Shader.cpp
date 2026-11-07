#include "Shader.hpp"
#include <exception>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

using namespace std;

ShaderException::ShaderException(ErrorCode err)
	: _errorCode(err) {}

const char *ShaderException::what() const noexcept {
	switch (_errorCode) {
		case TYPE_NOT_FOUND: return "Not existing shader type";
		case SOURCE_NOT_FOUND: return "Source for shader not found";
		case COMPILATION_FAILED: return "Shader compilation failed";
		case UNKNOWN_ERROR: return "Failed to get shader source";
		default: return "An unknown error occured during shader creating";
	}
}

Shader::Shader(const char *fileName, GLenum shaderType)
	: _shaderType(shaderType)
{
	if (fileName == nullptr || fileName[0] == '\0') {
		throw ShaderException(ShaderException::SOURCE_NOT_FOUND);
	}

	cout << "Creating shader from " << fileName << "..." << endl;

	if (!isValidShaderType(_shaderType)) {
		throw ShaderException(ShaderException::TYPE_NOT_FOUND);
	}

	_shaderSourceStr = getFileContent(fileName);
	_shaderSource = _shaderSourceStr.c_str();

	_shader = glCreateShader(_shaderType);
	glShaderSource(_shader, 1, &_shaderSource, NULL);
	glCompileShader(_shader);

	GLint isCompiled = 0;
	glGetShaderiv(_shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &maxLength);

		// the maxLength includes the NULL character
		vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(_shader, maxLength, &maxLength, &errorLog[0]);
		cerr << "Shader compilation failed: " << errorLog.data();

		// delete the shader if compilation failed
		glDeleteShader(_shader);
		throw ShaderException(ShaderException::COMPILATION_FAILED);
	}
}

GLuint Shader::getShader() const {
	return _shader;
}

void Shader::deleteShader() {
	glDeleteShader(_shader);
	cout << "Shader was deleted" << endl;
}

// private //

const GLenum Shader::_shaderTypes[5] = {
	GL_VERTEX_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER,
	GL_GEOMETRY_SHADER,
	GL_FRAGMENT_SHADER,
};

bool Shader::isValidShaderType(GLenum type) const {
	for (int i = 0; i < 5; ++i) {
		if (_shaderTypes[i] == type)
			return true;
	}
	return false;
}

string Shader::getFileContent(const char *fileName) {
	ifstream inputFile(fileName, ios::in);
	if (!inputFile) {
		throw ShaderException(ShaderException::UNKNOWN_ERROR);
	}

	ostringstream content;
	content << inputFile.rdbuf();
	return(content.str());
}
