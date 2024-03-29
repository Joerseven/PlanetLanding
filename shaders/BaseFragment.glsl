#version 330 core

uniform sampler2D diffuseTex;
uniform vec3 cameraPos;
uniform vec4 lightColor;
uniform vec3 lightPos;
uniform float lightRadius;

in Vertex {
    vec2 texCoord;
    vec4 colour;
    vec3 normal;
    vec3 worldPos;
} IN;

out vec4 fragColor;

vec4 addLight(vec3 baseColor) {

    vec3 incident = normalize(lightPos - IN.worldPos);
    vec3 viewDir = normalize(cameraPos - IN.worldPos);
    vec3 halfDir = normalize(incident + viewDir);

    float lambert = max(dot(incident, IN.normal), 0.0f);
    float pDistance = length(lightPos - IN.worldPos);
    float attenuation = 1.0 - clamp(pDistance / lightRadius, 0.0, 1.0);

    float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
    specFactor = pow(specFactor, 60.0);

    vec3 landColor = vec3(1.0, 1.0, 1.0) - IN.colour.rgb;

    vec3 o = baseColor * lambert * attenuation;
    o += (lightColor.rgb * specFactor)*attenuation*0.33;
    o += baseColor * 0.2f;
    return vec4(o, 1.0);
}

void main() {
    fragColor = addLight(IN.colour.xyz);
}
