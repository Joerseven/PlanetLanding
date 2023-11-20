#version 330 core

uniform sampler2D diffuseTex;
uniform vec3 cameraPos;
uniform vec4 lightColor;
uniform vec3 lightPos;
uniform float lightRadius;

uniform int glowFactor;
uniform vec3 viewVector;

in Vertex {
    vec2 texCoord;
    vec4 colour;
    vec3 normal;
    vec3 worldPos;
} IN;

out vec4 fragColor;

void main() {
    vec3 incident = normalize(lightPos - IN.worldPos);
    vec3 viewDir = normalize(cameraPos - IN.worldPos);
    vec3 halfDir = normalize(incident + viewDir);

    float lambert = max(dot(incident, IN.normal), 0.0f);
    float pDistance = length(lightPos - IN.worldPos);
    float attenuation = 1.0 - clamp(pDistance / lightRadius, 0.0, 1.0);

    float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
    specFactor = pow(specFactor, 60.0);

    vec3 landColor = vec3(1.0, 1.0, 1.0) - IN.colour.rgb;

    vec2 tC = vec2(IN.texCoord.x * 0.1, IN.texCoord.y);
    vec3 unlitSea = IN.colour.rgb * step(0.5, texture(diffuseTex, tC).r);
    vec3 unlitLand = landColor * (1-step(0.5, texture(diffuseTex, tC).r));

    //vec3 surface = (unlitSea + unlitLand) * lightColor.rgb;
    vec3 surface = texture(diffuseTex, IN.texCoord).rgb;
    fragColor.rgb = surface * lambert * attenuation;
    fragColor.rgb += (lightColor.rgb * specFactor)*attenuation*0.33;
    fragColor.rgb += surface * 0.25f;
    fragColor.rgb += vec3(pow((1.0 - clamp(dot(IN.normal, viewDir), 0.0, 1.0)), 3.0)) * glowFactor * 2; // selection glow
    fragColor.a = 1.0;
}
