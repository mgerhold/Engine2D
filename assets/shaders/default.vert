#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in uint aTexIndex;

out vec4 fragmentColor;
out vec3 fragmentPosition;
out vec2 texCoords;
flat out uint texIndex;

uniform mat4 projectionMatrix;

void main() {
   vec4 position = projectionMatrix * vec4(aPos.xyz, 1.0);
   fragmentPosition = position.xyz;
   fragmentColor = aColor;
   texCoords = aTexCoords;
   texIndex = aTexIndex;
   gl_Position = position;
}