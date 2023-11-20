#version 330 core
in vec3 position;
in vec3 normal;
in vec4 colour;

out Vertex {
    vec3 normal;
    vec3 position;
    vec4 color;
} OUT;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main()
{
    OUT.color = colour;
    OUT.normal = mat3(transpose(inverse(modelMatrix))) * normal;
    OUT.position = vec3(modelMatrix * vec4(position, 1.0));
    gl_Position = projMatrix * viewMatrix * vec4(OUT.position, 1.0);
}