#version 330 core

uniform sampler2D srcTexture;
uniform float filterRadius;

in Vertex {
    vec2 texCoord;
} IN;

out vec3 upsample;

void main() {
    float x = filterRadius;
    float y = filterRadius;

    vec3 a = texture(srcTexture, vec2(IN.texCoord.x - x, IN.texCoord.y + y)).rgb;
    vec3 b = texture(srcTexture, vec2(IN.texCoord.x,     IN.texCoord.y + y)).rgb;
    vec3 c = texture(srcTexture, vec2(IN.texCoord.x + x, IN.texCoord.y + y)).rgb;

    vec3 d = texture(srcTexture, vec2(IN.texCoord.x - x, IN.texCoord.y)).rgb;
    vec3 e = texture(srcTexture, vec2(IN.texCoord.x,     IN.texCoord.y)).rgb;
    vec3 f = texture(srcTexture, vec2(IN.texCoord.x + x, IN.texCoord.y)).rgb;

    vec3 g = texture(srcTexture, vec2(IN.texCoord.x - x, IN.texCoord.y - y)).rgb;
    vec3 h = texture(srcTexture, vec2(IN.texCoord.x,     IN.texCoord.y - y)).rgb;
    vec3 i = texture(srcTexture, vec2(IN.texCoord.x + x, IN.texCoord.y - y)).rgb;

    upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;

}
