#include "MatrixStack.hpp"
#include "MatrixTransform.hpp"

#include <algorithm>

MatrixStack::MatrixStack() {
    reset();
}

void MatrixStack::reset() {
    _size = 1;
    MatrixTransform::loadIdentity(_entries[0].data());
}

bool MatrixStack::push() {
    if (_size == capacity) return false;
    _entries[_size] = _entries[_size - 1];
    ++_size;
    return true;
}

bool MatrixStack::pop() {
    if (_size == 1) return false;
    --_size;
    return true;
}

std::size_t MatrixStack::size() const {
    return _size;
}

MatrixStack::Matrix MatrixStack::top() const {
    return _entries[_size - 1];
}

void MatrixStack::load(const Matrix &matrix) {
    load(matrix.data());
}

void MatrixStack::load(const GLfloat *matrix) {
    if (!matrix) throw MatrixTransformException(MatrixTransformException::INVALID_MATRIX);
    std::copy_n(matrix, 16, _entries[_size - 1].begin());
}

void MatrixStack::multiply(const Matrix &local) {
    multiply(local.data());
}

void MatrixStack::multiply(const GLfloat *local) {
    MatrixTransform::multiply(_entries[_size - 1].data(), local,
                              _entries[_size - 1].data());
}

void MatrixStack::translate(GLfloat x, GLfloat y, GLfloat z) {
    Matrix local;
    MatrixTransform::loadIdentity(local.data());
    MatrixTransform::translate(local.data(), x, y, z);
    multiply(local);
}

void MatrixStack::rotateX(GLfloat degrees) {
    Matrix local;
    MatrixTransform::loadIdentity(local.data());
    MatrixTransform::rotateX(local.data(), degrees);
    multiply(local);
}

void MatrixStack::rotateY(GLfloat degrees) {
    Matrix local;
    MatrixTransform::loadIdentity(local.data());
    MatrixTransform::rotateY(local.data(), degrees);
    multiply(local);
}

void MatrixStack::rotateZ(GLfloat degrees) {
    Matrix local;
    MatrixTransform::loadIdentity(local.data());
    MatrixTransform::rotateZ(local.data(), degrees);
    multiply(local);
}

void MatrixStack::scale(GLfloat x, GLfloat y, GLfloat z) {
    Matrix local;
    MatrixTransform::loadIdentity(local.data());
    MatrixTransform::scale(local.data(), x, y, z);
    multiply(local);
}

void MatrixStack::scale(GLfloat factor) {
    scale(factor, factor, factor);
}
