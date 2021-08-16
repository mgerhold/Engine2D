#version 430 core

in vec3 fragmentPosition;
in vec4 fragmentColor;
in vec2 texCoords;
flat in uint texIndex;

out vec4 FragColor;

layout (binding = 0) uniform sampler2D uTextures[32];

void main() {
    vec4 color = texture(uTextures[texIndex], texCoords) * fragmentColor;
    if (color.a == 0.0) {
        discard;
    }
    FragColor = color;
}