#version 330

uniform sampler2D texture0;
uniform vec3 testSpherePoint;

in Vertex {
    vec2 texCoord;
} IN;

out vec3 fragColor;

void main() {
    fragColor.rgb = vec3(0.9, 0.1, 0.1);
}
