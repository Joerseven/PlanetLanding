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

    vec3 surface = ((IN.colour.rgb * step(0.5, texture(diffuseTex, IN.texCoord).r)) * lightColor.rgb) + (landColor * (1-smoothstep(0.5, 0.51, texture(diffuseTex, IN.texCoord).r) * lightColor.rgb));
    //vec3 surface = IN.colour.rgb;
    fragColor.rgb = surface * lambert * attenuation;
    fragColor.rgb += (lightColor.rgb * specFactor)*attenuation*0.33;
    fragColor.rgb += surface * 0.8f;
    fragColor.a = 1.0;
}
