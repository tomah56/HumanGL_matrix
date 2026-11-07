#include "BMPLoader.hpp"
#include "../ResourcePath.hpp"
#include <exception>
#include <string>
#include <stdio.h>
#include <vector>
#include <iostream>

using namespace std;

BMPLoaderException::BMPLoaderException(ErrorCode err, const string &path)
	: _errorCode(err), _path(path)
{
	if (_path.empty()) {
		_path = "no path given";
	}

	switch(_errorCode) {
		case FILE_NOT_FOUND:
			_returnMessage = "File with rendering model not found: " + _path;
			break;
		case CANNOT_OPEN:
			_returnMessage = "File with rendering model cannot be opened: " + _path;
			break;
		case INVALID_FORMAT:
			_returnMessage = "Invalid file format: " + _path;
			break;
		default:
			_returnMessage = "An unknown error occurred during BMP image loading: " + _path;
			break;
	}
}

const char *BMPLoaderException::what() const noexcept {
	return _returnMessage.c_str();
}

BMPLoader::BMPLoader(const string &path)
	: _path(path)
{
	if (_path.empty()) {
		throw BMPLoaderException(BMPLoaderException::FILE_NOT_FOUND, _path);
	}

	FILE* file = fopen(ResourcePath::getPath(path).c_str(), "rb");
	if (!file) {
		throw BMPLoaderException(BMPLoaderException::CANNOT_OPEN, _path);
	}

	fread(&_bmpHeader, sizeof(Header), 1, file);

	if (_bmpHeader.signature != 0x4D42) {
		fclose(file);
		throw BMPLoaderException(BMPLoaderException::INVALID_FORMAT, _path);
	}

	size_t pixelDataSize = _bmpHeader.fileSize - _bmpHeader.dataOffset;
	_pixelData.resize(pixelDataSize);  // Allocate memory in the vector
	fseek(file, _bmpHeader.dataOffset, SEEK_SET);
	fread(_pixelData.data(), 1, pixelDataSize, file);
	if (_pixelData.empty()) {
		throw BMPLoaderException(BMPLoaderException::INVALID_FORMAT, _path);
	}
	fclose(file);

	channels = _bmpHeader.bitsPerPixel / 8;
}

const Header *BMPLoader::getBMPHeader() const {
	return &_bmpHeader;
}

const unsigned char *BMPLoader::getPixelData() const {
	return _pixelData.data();
}
