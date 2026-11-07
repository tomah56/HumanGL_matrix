#include "Texture.hpp"
#include "BMPLoader.hpp"
#include <glad/gl.h>
#include <exception>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>

using namespace std;

Texture::Texture(const string &path) :
	_bmpImage(path)
{
	glGenTextures(1, &_textureID);
	glBindTexture(GL_TEXTURE_2D, _textureID);

	GLenum formatBMPImage =(_bmpImage.channels == 4) ? GL_BGRA : GL_BGR;

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB,
		_bmpImage.getBMPHeader()->width,
		_bmpImage.getBMPHeader()->height,
		0, formatBMPImage, GL_UNSIGNED_BYTE,
		_bmpImage.getPixelData());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT) ;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT) ;

	glActiveTexture(GL_TEXTURE0);
}

// other

void Texture::unbindTexture() {
	glBindTexture(GL_TEXTURE_2D, 0);
}
