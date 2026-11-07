#include "Render.hpp"

void Render::beginFrame(const Camera &camera) const {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(_shaderProgram.getShaderProgram());
    GLint texture = 0;
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
    if (texture == 0) glBindTexture(GL_TEXTURE_2D, _whiteTexture);
    glUniformMatrix4fv(_uniformLocations.viewMatrix, 1, GL_TRUE, camera.viewMatrix);
    glUniformMatrix4fv(_uniformLocations.projectionMatrix, 1, GL_TRUE, camera.projectionMatrix);
}

void Render::drawUnitCube(const MatrixStack &stack, const std::array<GLfloat, 3> &color) const {
    const MatrixStack::Matrix model = stack.top();
    glUseProgram(_shaderProgram.getShaderProgram());
    glUniformMatrix4fv(_uniformLocations.modelMatrix, 1, GL_TRUE, model.data());
    glUniform1i(_uniformLocations.useSolidColor, GL_TRUE);
    glUniform3fv(_uniformLocations.solidColor, 1, color.data());
    glBindVertexArray(_cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

void Render::drawMesh(const GLfloat *modelMatrix, const Material &material) const {
    if (!modelMatrix) throw MatrixTransformException(MatrixTransformException::INVALID_MATRIX);
    if (_mesh.indices.empty()) throw RenderException(RenderException::ERR);
    glUseProgram(_shaderProgram.getShaderProgram());
    glUniformMatrix4fv(_uniformLocations.modelMatrix, 1, GL_TRUE, modelMatrix);
    glUniform1i(_uniformLocations.useSolidColor, GL_FALSE);
    glUniform1i(_uniformLocations.texture, 0);
    glUniform1f(_uniformLocations.mixValue, material.materialBlend.mixValue);
    glUniform1i(_uniformLocations.useFaceColors, material.materialBlend.faceColorMode);
    glBindVertexArray(_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_mesh.indices.size()), GL_UNSIGNED_INT, nullptr);
}
