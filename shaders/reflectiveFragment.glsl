#version 330 core
out vec4 fragColor;

in Vertex {
    vec3 normal;
    vec3 position;
    vec4 color;
} IN;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{
    vec3 I = normalize(IN.position - cameraPos);
    vec3 r = reflect(I, normalize(IN.normal));
    fragColor = vec4(texture(skybox, r).rgb + IN.color.rgb, 1.0);
}