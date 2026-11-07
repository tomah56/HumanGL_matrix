#pragma once

#include <cstdint>
#include <exception>
#include <string>
#include <vector>

class BMPLoaderException : public std::exception {
public:
	enum ErrorCode {
		FILE_NOT_FOUND,
		CANNOT_OPEN,
		INVALID_FORMAT,
	};

	explicit BMPLoaderException(ErrorCode err, const std::string &path);
	const char *what() const noexcept override;

private:
	ErrorCode _errorCode;
	std::string _path;
	std::string _returnMessage;
};

#pragma pack(push, 1)
struct Header {
    std::uint16_t signature;      // 0x4D42
    std::uint32_t fileSize;
    std::uint16_t reserved1;
    std::uint16_t reserved2;
    std::uint32_t dataOffset;

    std::uint32_t size;           // info header size
    std::uint32_t width;
    std::uint32_t height;
    std::uint16_t planes;
    std::uint16_t bitsPerPixel;
    std::uint32_t compression;
    std::uint32_t imageSize;
    std::uint32_t horizontalResolution;
    std::uint32_t verticalResolution;
    std::uint32_t colorsUsed;
    std::uint32_t importantColors;
};
#pragma pack(pop)

class BMPLoader {
public:
	int channels = 0;

	explicit BMPLoader(const std::string &path);

	const Header *getBMPHeader() const;
	const unsigned char *getPixelData() const;

private:
	std::string _path;
	Header _bmpHeader;
	std::vector<unsigned char> _pixelData; // BGR

	BMPLoader();
};
