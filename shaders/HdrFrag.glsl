#version 330 core

out vec4 fragColor;

uniform sampler2D hdrBuffer;

in Vertex {
    vec2 texCoord;
} IN;

mat3 aInputMat = mat3(
    0.59719,0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
);

mat3 aOutputMat = mat3(
     1.60475, -0.10208, -0.00327,
    -0.53108,  1.10813, -0.07276,
    -0.07367, -0.00605,  1.07602
);

vec3 wtfIsThis(vec3 v) {
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 aces(vec3 color)
{
    color = aInputMat * color;

    color = wtfIsThis(color);

    color = aOutputMat * color;

    // Clamp to [0, 1]
    color = clamp(color, 0.0, 1.0);

    return color;
}

void main() {
    vec3 hdrColor = texture(hdrBuffer, IN.texCoord).rgb;
    fragColor = vec4(aces(hdrColor), 1.0);
}
