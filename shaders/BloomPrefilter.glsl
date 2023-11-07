#version 330 core

in Vertex {
    vec2 texCoord;
} IN;


out vec3 prefilter;

uniform sampler2D srcTexture;

const float threshold = 1.0;

void main() {
    vec3 c = texture(srcTexture, IN.texCoord).rgb;
    float brightness = max(c.r, max(c.g, c.b));
    float cont = max(0, brightness - threshold);
    cont /= max(brightness, 0.00001);
    prefilter = c * cont;
}
