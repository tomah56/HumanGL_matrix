#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

struct MaterialBlend {
	// Switch texture/color
    bool textureMode = false;
	bool faceColorMode = false;
	GLfloat mixValue = 0.0f;  // 0.0 = colors, 1.0 = texture
	GLfloat targetMixValue = 0.0f;
	GLfloat transitionSpeed = 3.0f;
};

class Material {
public:
	MaterialBlend materialBlend;
};
