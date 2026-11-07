#pragma once

#include "../shaders/ShaderProgram.hpp"
#include "../modelLoader/RenderModelLoader.hpp"
#include "../scene/Transformation.hpp"
#include "../scene/Camera.hpp"
#include "../scene/Material.hpp"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <exception>
#include <vector>
#include <unordered_map>
#include <string>

class RenderException : public std::exception {
public:
	enum ErrorCode {
		ERR,
	};

	explicit RenderException(ErrorCode err);
	const char *what() const noexcept override;

private:
	ErrorCode _errorCode;
};

struct ShaderUniforms {
	GLint texture;
    GLint mixValue;
    GLint useFaceColors;
    GLint modelMatrix;
    GLint viewMatrix;
    GLint projectionMatrix;
};

class Render {
public:
	explicit Render(const Mesh &mesh, ShaderProgram &shaderProgram);

	void uploadUniforms();
	const ShaderUniforms &getUniformLocation() const;
	GLuint getVAO() const;
	void glSettings();
	void cleanUp();

	void renderFrame(double deltaTime, Transformation &transformation, Camera &camera, Material &material);

private:
	Mesh _mesh;
	ShaderProgram _shaderProgram;
	GLuint _VAO, _VBO, _EBO;
	ShaderUniforms _uniformLocations;

	Render();
};
