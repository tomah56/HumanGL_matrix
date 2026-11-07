#include "Render.hpp"
#include "UnitCube.hpp"
#include <exception>
#include <vector>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "../shaders/ShaderProgram.hpp"
#include "../modelLoader/RenderModelLoader.hpp"

using namespace std;

RenderException::RenderException(ErrorCode err)
	: _errorCode(err) {}

const char *RenderException::what() const noexcept {
	switch (_errorCode) {
		case ERR: return "Render error";
		default: return "An unknown error occured during shader creating";
	}
}

Render::Render(ShaderProgram &shaderProgram)
	: _shaderProgram(shaderProgram)
{
	uploadMesh(makeUnitCubeMesh(), _cubeVAO, _cubeVBO, _cubeEBO);
	// Keep the legacy sampler complete even when the shader uses only solid color.
	const GLubyte white[] = {255, 255, 255, 255};
	GLint previousTexture = 0;
	glActiveTexture(GL_TEXTURE0);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
	glGenTextures(1, &_whiteTexture);
	glBindTexture(GL_TEXTURE_2D, _whiteTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glSettings();
	uploadUniforms();
}

Render::Render(const Mesh &mesh, ShaderProgram &shaderProgram)
	: Render(shaderProgram)
{
	_mesh = mesh;
	if (!_mesh.indices.empty()) uploadMesh(_mesh, _VAO, _VBO, _EBO);
}

void Render::uploadMesh(const Mesh &mesh, GLuint &vao, GLuint &vbo, GLuint &ebo) {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mesh.vertices.size(), mesh.vertices.data(), GL_STATIC_DRAW);

	int stride = 11 * sizeof(GLfloat);

	// Position attribute (location = 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	// Texture coord attribute (location = 1)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Vertex color attribute (location = 2)
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Face color attribute (location = 3)
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.indices.size(), mesh.indices.data(), GL_STATIC_DRAW);

	// Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// getters //

const ShaderUniforms &Render::getUniformLocation() const {
	return _uniformLocations;
}

GLuint Render::getVAO() const {
	return _VAO;
}

// other //

void Render::uploadUniforms() {
	// check for -1
	_uniformLocations.texture = glGetUniformLocation(_shaderProgram.getShaderProgram(), "tex");
	_uniformLocations.mixValue = glGetUniformLocation(_shaderProgram.getShaderProgram(), "mixValue");
	_uniformLocations.useFaceColors = glGetUniformLocation(_shaderProgram.getShaderProgram(), "useFaceColors");
	_uniformLocations.modelMatrix = glGetUniformLocation(_shaderProgram.getShaderProgram(), "modelMatrix");
	_uniformLocations.viewMatrix = glGetUniformLocation(_shaderProgram.getShaderProgram(), "viewMatrix");
	_uniformLocations.projectionMatrix = glGetUniformLocation(_shaderProgram.getShaderProgram(), "projectionMatrix");
	_uniformLocations.useSolidColor = glGetUniformLocation(_shaderProgram.getShaderProgram(), "useSolidColor");
	_uniformLocations.solidColor = glGetUniformLocation(_shaderProgram.getShaderProgram(), "solidColor");
}

void Render::glSettings() {
	// the depth value for every pixel
	// when enabled: performing the depth test
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_CULL_FACE);
	// glEnable(GL_CULL_FACE);
	// glFrontFace(GL_CW);
	// glCullFace(GL_BACK);
}

void Render::cleanUp() {
	glDeleteVertexArrays(1, &_VAO);
	glDeleteBuffers(1, &_EBO);
	glDeleteBuffers(1, &_VBO);
	glDeleteVertexArrays(1, &_cubeVAO);
	glDeleteBuffers(1, &_cubeEBO);
	glDeleteBuffers(1, &_cubeVBO);
	glDeleteTextures(1, &_whiteTexture);
	_VAO = _VBO = _EBO = 0;
	_cubeVAO = _cubeVBO = _cubeEBO = 0;
	_whiteTexture = 0;
}
