
/**
* @file RenderModelLoader.hpp
* @brief Minimal Wavefront OBJ loader for OpenGL rendering.
*
* This module parses geometry data from an OBJ file and prepares
* vertex and index buffers suitable for OpenGL rendering.
*
* Supported OBJ elements:
*  - v   (vertex positions)
*  - vt  (texture coordinates)
*  - vn  (normal vectors)
*  - f   (faces, triangulated using triangle fan)
*
* Unsupported / ignored:
*  - Materials (.mtl)
*  - Multiple objects / groups
*  - Negative indices
*/

#pragma once

#include <vector>
#include <string>
#include <exception>
#include <glad/gl.h>
#include <sstream>

/**
* @class RenderModelLoaderException
* @brief Exception type used for OBJ model loading errors.
*
* Provides categorized error reporting for file access
* and parsing failures.
*/
class RenderModelLoaderException : public std::exception {
public:
	enum ErrorCode {
		FILE_NOT_FOUND,
        CANNOT_OPEN,
        INVALID_FACE_FORMAT,
		INDEX_OUT_OF_RANGE,
        UNKNOWN_ERROR,
	};

	explicit RenderModelLoaderException(ErrorCode err);
	const char *what() const noexcept override;

private:
	ErrorCode _errorCode;
};


struct BoundingBox {
	GLfloat minX = 1e10f, maxX = -1e10f;
	GLfloat minY = 1e10f, maxY = -1e10f;
	GLfloat minZ = 1e10f, maxZ = -1e10f;

	GLfloat getRangeX() const {
		GLfloat r = maxX - minX;
		return (r < 0.0001f) ? 1.0f : r;
	}

	GLfloat getRangeY() const {
		GLfloat r = maxY - minY;
		return (r < 0.0001f) ? 1.0f : r;
	}

	GLfloat getRangeZ() const {
		GLfloat r = maxZ - minZ;
		return (r < 0.0001f) ? 1.0f : r;
	}
};

struct RawOBJData {
    std::vector<GLfloat> vertices;   // x,y,z,w per vertex
    std::vector<GLfloat> texCoords;  // u,v per texcoord
    std::vector<GLfloat> normals;    // x,y,z per normal
    std::vector<GLuint> vIndices;    // vertex indices from faces
    std::vector<GLuint> vtIndices;   // texcoord indices from faces
    std::vector<GLuint> vnIndices;   // normal indices from faces

    bool hasTexCoords() const { return !texCoords.empty() && !vtIndices.empty(); }
    bool hasNormals() const { return !normals.empty() && !vnIndices.empty(); }
};

struct Mesh {
    std::vector<GLfloat> vertices;   // interleaved [x,y,z,u,v,r_v,g_v,b_v,r_f,g_f,b_f]
    std::vector<GLuint> indices;     // deduplicated indices
};

/**
* @class RenderModelLoader
* @brief Represents a renderable 3D model loaded from an OBJ file.
*
* The class loads vertex data, texture coordinates, normals,
* and face indices from a Wavefront OBJ file.
*
* Faces with more than three vertices are automatically
* triangulated using a triangle fan approach.
*
* Vertex data is stored separately from index buffers,
* enabling indexed rendering (e.g. via OpenGL EBOs).
*/
class RenderModelLoader {
public:
    explicit RenderModelLoader(const std::string &path);

    const Mesh &getMesh() const;

private:
    Mesh _mesh;
    RawOBJData _raw;
    BoundingBox _bbox;
    std::string _path;

    GLfloat _posX;
    GLfloat _posY;
    GLfloat _posZ;
    GLfloat _nx;
    GLfloat _ny;
    GLfloat _nz;

    void parseOBJFile();
    void parseFaces(std::istringstream &line);
    void buildMesh();
    void calculateNormals();
    void calculateBoundingBox();
    void calculateUVCoordinates(GLuint vtIdx, GLuint vnIdx);
    void calculateVertexColor(GLuint vnIdx);
    void calculateCubicUV();
    void centerVertices();

    RenderModelLoader();
};
