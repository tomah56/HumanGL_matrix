#pragma once

#include "BMPLoader.hpp"
#include <exception>
#include <glad/gl.h>
#include <string>

class Texture {
public:
	explicit Texture(const std::string &path);

	void unbindTexture();

private:
	BMPLoader _bmpImage;
	GLuint _textureID;

	Texture();
};
