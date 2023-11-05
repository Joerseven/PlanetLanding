#version 330 core

out vec4 fragColor;

in Vertex {
    vec2 texCoord;
} IN;

uniform sampler2D screenTexture;

void main() {
    fragColor = texture(screenTexture, texCoords);
}
