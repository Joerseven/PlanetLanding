#version 330 core

uniform sampler2D srcTexture;
uniform vec2 srcResolution;

in Vertex {
    vec2 texCoord;
} IN;

out vec3 downsample;

void main() {
    vec2 srcTexelSize = 1.0 / srcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    vec3 a = texture(srcTexture, vec2(IN.texCoord.x - 2*x, IN.texCoord.y + 2*y)).rgb;
    vec3 b = texture(srcTexture, vec2(IN.texCoord.x,       IN.texCoord.y + 2*y)).rgb;
    vec3 c = texture(srcTexture, vec2(IN.texCoord.x + 2*x, IN.texCoord.y + 2*y)).rgb;

    vec3 d = texture(srcTexture, vec2(IN.texCoord.x - 2*x, IN.texCoord.y)).rgb;
    vec3 e = texture(srcTexture, vec2(IN.texCoord.x,       IN.texCoord.y)).rgb;
    vec3 f = texture(srcTexture, vec2(IN.texCoord.x + 2*x, IN.texCoord.y)).rgb;

    vec3 g = texture(srcTexture, vec2(IN.texCoord.x - 2*x, IN.texCoord.y - 2*y)).rgb;
    vec3 h = texture(srcTexture, vec2(IN.texCoord.x,       IN.texCoord.y - 2*y)).rgb;
    vec3 i = texture(srcTexture, vec2(IN.texCoord.x + 2*x, IN.texCoord.y - 2*y)).rgb;

    vec3 j = texture(srcTexture, vec2(IN.texCoord.x - x, IN.texCoord.y + y)).rgb;
    vec3 k = texture(srcTexture, vec2(IN.texCoord.x + x, IN.texCoord.y + y)).rgb;
    vec3 l = texture(srcTexture, vec2(IN.texCoord.x - x, IN.texCoord.y - y)).rgb;
    vec3 m = texture(srcTexture, vec2(IN.texCoord.x + x, IN.texCoord.y - y)).rgb;

    downsample = e*0.125;
    downsample += (a+c+g+i)*0.03125;
    downsample += (b+d+f+h)*0.0625;
    downsample += (j+k+l+m)*0.125;
}
