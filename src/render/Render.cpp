#include "Render.hpp"
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

Render::Render(const Mesh &mesh, ShaderProgram &shaderProgram)
	:  _mesh(mesh), _shaderProgram(shaderProgram)
{

	// These commands set up the coordinates of the triangle to be rendered.
	// They tell OpenGL the location in memory that the positions of the triangle will come from

	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO);

	glGenBuffers(1, &_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _mesh.vertices.size(), _mesh.vertices.data(), GL_STATIC_DRAW);

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

	glGenBuffers(1, &_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * _mesh.indices.size(), _mesh.indices.data(), GL_STATIC_DRAW);

	// Sets the color that will be used when clearing the screen
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glSettings();

	// Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	uploadUniforms();
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
	_shaderProgram.deleteShaderProgram();
}
