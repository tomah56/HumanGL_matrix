#pragma once

#include "../shaders/ShaderProgram.hpp"
#include "../modelLoader/RenderModelLoader.hpp"
#include "../scene/Transformation.hpp"
#include "../scene/Camera.hpp"
#include "../scene/Material.hpp"
#include "../matrixMath/MatrixStack.hpp"
#include <array>
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
    GLint useSolidColor;
    GLint solidColor;
};

class Render {
public:
	explicit Render(ShaderProgram &shaderProgram);
	explicit Render(const Mesh &mesh, ShaderProgram &shaderProgram);
	Render(const Render &) = delete;
	Render &operator=(const Render &) = delete;

	void uploadUniforms();
	const ShaderUniforms &getUniformLocation() const;
	GLuint getVAO() const;
	void glSettings();
	void cleanUp();

	// Call once per frame, then draw any number of parts using the shared cube.
	void beginFrame(const Camera &camera) const;
	void drawUnitCube(const MatrixStack &stack, const std::array<GLfloat, 3> &color) const;
	void drawMesh(const GLfloat *modelMatrix, const Material &material) const;

private:
	Mesh _mesh;
	ShaderProgram &_shaderProgram; // Borrowed; the caller owns the program.
	GLuint _VAO = 0, _VBO = 0, _EBO = 0;
	GLuint _cubeVAO = 0, _cubeVBO = 0, _cubeEBO = 0;
	GLuint _whiteTexture = 0;
	ShaderUniforms _uniformLocations;

	Render();
	static void uploadMesh(const Mesh &mesh, GLuint &vao, GLuint &vbo, GLuint &ebo);
};
