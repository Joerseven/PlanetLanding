#version 330


in Vertex {
    vec2 texCoord;
} IN;


out vec3 postfilter;

uniform sampler2D srcTex;
uniform sampler2D bloomTex;

void main() {
    postfilter = texture(srcTex, IN.texCoord).rgb + texture(bloomTex, IN.texCoord).rgb;
}
