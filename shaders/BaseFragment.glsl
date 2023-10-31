#version 330 core

uniform sampler2D diffuseTex;

in Vertex {
    vec2 texCoord;
    vec4 colour;
} IN;

out vec4 fragColor;

void main() {
    fragColor = IN.colour; // texture(diffuseTex, IN.texCoord) *
}
