#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;
in vec4 colour;
in vec2 texCoord;
in vec3 normal;

out Vertex {
    vec2 texCoord;
    vec4 colour;
    vec3 normal;
    vec3 worldPos;
} OUT;

void main() {

    OUT.texCoord = texCoord;
    OUT.colour = colour;
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    OUT.normal = normalize(normalMatrix * normalize(normal));
    vec4 worldPos = (modelMatrix * vec4(position, 1));
    OUT.worldPos = worldPos.xyz;
    vec2 uv = vec2(clamp(((atan(position.z, position.x) / radians(180.0)) + 1.0) / 2.0, 0.0, 1.0), (0.5-(asin(position.y)/radians(180.0))));
    OUT.texCoord = uv;
    gl_Position = (projMatrix * viewMatrix) * worldPos;
}
