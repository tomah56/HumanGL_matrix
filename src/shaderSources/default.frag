#version 410 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 VertexColor;
in vec3 FaceColor;

uniform sampler2D tex;
uniform float mixValue;  // 0.0 = colors, 1.0 = texture
uniform bool useFaceColors;  // true = face colors, false = vertex colors

void main() {
    vec4 colorFromVertices = vec4(VertexColor, 1.0);
    vec4 colorFromFaces = vec4(FaceColor, 1.0);
    vec4 colorFromTexture = texture(tex, TexCoord);

    vec4 colorSource = useFaceColors ? colorFromFaces : colorFromVertices;

    FragColor = mix(colorSource, colorFromTexture, mixValue);
}