#version 430 core

in vec3 fragmentPosition;
in vec4 fragmentColor;
in vec2 texCoords;
flat in uint texIndex;

out vec4 FragColor;

layout (binding = 0) uniform sampler2D uTextures[32];

void main() {
    vec4 color = vec4(fragmentPosition.xy, 0.0, 1.0);
    FragColor = color;
}