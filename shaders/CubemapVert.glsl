#version 330 core

in vec3 position;

out vec3 texCoords;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main() {
    texCoords = position;
    gl_Position = projMatrix * viewMatrix * vec4(position, 1.0);
}