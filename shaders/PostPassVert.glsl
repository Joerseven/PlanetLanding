#version 330 core
in vec3 position;
in vec2 texCoord;

out Vertex {
    vec2 texCoord;
    vec4 colour;
} OUT;

void main() {
    gl_Position = vec4(position.x, position.y, 0, 1.0);
    OUT.texCoord = texCoord;
}
