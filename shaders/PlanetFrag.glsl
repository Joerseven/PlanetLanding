#version 330 core

uniform sampler2D diffuseTex;

in Vertex {
    vec2 texCoord;
    vec4 colour;
    vec3 normal;
} IN;

out vec4 fragColor;

void main() {
    fragColor = vec4(IN.normal, 1.0); // texture(diffuseTex, IN.texCoord) *
}
