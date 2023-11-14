#version 330 core

out vec4 fragColor;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

uniform vec3 cameraPosition;
uniform vec3 planetCenter;
uniform mat4 projViewMatrix;
uniform mat4 projMatrix;
uniform mat4 viewMatrix;


in Vertex {
    vec2 texCoord;
    vec3 screenPos;
} IN;

const float atmosphereRadius = 0.6f;

vec2 raySphere(vec3 sphereCenter, float sphereRadius, vec3 rayOrigin, vec3 rayDirection) {
    vec3 offset = rayOrigin - sphereCenter;
    float a = dot(rayDirection, rayDirection);
    float b = 2 * dot(offset, rayDirection);
    float c = dot(offset, offset) - sphereRadius * sphereRadius;
    float d = b * b - 4 * a * c;

    if (d > 0) {
        float s = sqrt(d);
        float dstToSphereNear = max(0, (-b - s) / (2 * a));
        float dstToSphereFar = (-b + s) / (2 * a);

        if (dstToSphereFar >= 0) {
            return vec2(dstToSphereNear, dstToSphereFar - dstToSphereNear);
        }
    }

    return vec2(1.0 / 0.0, 0);
}

float near = 0.1;
float far = 100.0;

float linearDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
    vec4 originalColor = texture(screenTexture, IN.texCoord);
    float depth = linearDepth(texture(depthTexture, IN.texCoord).r) / far;

    vec3 rayOrigin = cameraPosition;
    vec4 rayDir = inverse(projViewMatrix) * vec4(IN.screenPos.xy, 1.0, 1.0);
    rayDir.xyz /= rayDir.w;


    float sceneDepth = length(inverse(projViewMatrix) * vec4(IN.screenPos.xy, 1.0, 1.0));

    vec4 viewNormal = normalize(inverse(projViewMatrix) * vec4(IN.screenPos.xy, 1.0, 1.0));
    vec2 hitInfo = raySphere(planetCenter, atmosphereRadius, rayOrigin, normalize(rayDir.xyz));

    float dstToAtmosphere = hitInfo.x;
    float dstThroughAtmosphere = min(hitInfo.y, depth);

    fragColor = vec4(vec3(dstThroughAtmosphere / (atmosphereRadius * 2)) * vec3(rayDir.rgb), 1.0);
}
