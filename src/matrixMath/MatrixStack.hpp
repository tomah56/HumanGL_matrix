#pragma once

#include <array>
#include <cstddef>
#include <glad/gl.h>

// Row-major model matrices. The base entry always exists.
class MatrixStack {
public:
    using Matrix = std::array<GLfloat, 16>;
    static constexpr std::size_t capacity = 32;

    MatrixStack();
    void reset();
    bool push();
    bool pop();
    std::size_t size() const;
    Matrix top() const;
    void load(const Matrix &matrix);
    void load(const GLfloat *matrix);
    void multiply(const Matrix &local);
    void multiply(const GLfloat *local);

    // Compose local transforms: top = top * local. Rotation angles are degrees.
    void translate(GLfloat x, GLfloat y, GLfloat z);
    void rotateX(GLfloat degrees);
    void rotateY(GLfloat degrees);
    void rotateZ(GLfloat degrees);
    void scale(GLfloat x, GLfloat y, GLfloat z);
    void scale(GLfloat factor);

private:
    std::array<Matrix, capacity> _entries = {};
    std::size_t _size = 1;
};
