#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec3 aFaceColor;

out vec2 TexCoord;
out vec3 VertexColor;
out vec3 FaceColor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);
    TexCoord = aTex;
    VertexColor = aColor;
    FaceColor = aFaceColor;
}