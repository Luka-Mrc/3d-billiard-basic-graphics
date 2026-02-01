#version 330 core

// Input from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

// Output color
out vec4 FragColor;

// Uniforms
uniform vec3 uObjectColor;    // Base color of the object
uniform vec3 uLightPos;       // Light position in world space
uniform vec3 uLightColor;     // Light color
uniform vec3 uViewPos;        // Camera position for specular
uniform float uAmbient;       // Ambient light intensity
uniform float uSpecular;      // Specular intensity
uniform float uShininess;     // Specular shininess exponent
uniform sampler2D uShadowMap; // Shadow depth map

float ShadowCalculation(vec4 fragPosLS)
{
    // Perspective divide
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    // Transform from [-1,1] to [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    // Fragment is outside the shadow map
    if (projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;

    // Bias based on surface angle to light
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    float bias = max(0.003 * (1.0 - dot(norm, lightDir)), 0.001);

    // PCF 5x5 for soft shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    for (int x = -2; x <= 2; ++x)
    {
        for (int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;

    return shadow;
}

void main()
{
    // Normalize the normal
    vec3 norm = normalize(Normal);

    // Point light direction
    vec3 lightDir = normalize(uLightPos - FragPos);

    // Distance attenuation (very gentle, so overhead light covers the whole table evenly)
    float dist = length(uLightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.007 * dist + 0.002 * dist * dist);

    // ============ Ambient ============
    vec3 ambient = uAmbient * uLightColor;

    // ============ Diffuse ============
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * attenuation;

    // ============ Specular (Blinn-Phong) ============
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), uShininess);
    vec3 specular = uSpecular * spec * uLightColor * attenuation;

    // ============ Shadow ============
    float shadow = ShadowCalculation(FragPosLightSpace);

    // Shadow darkens diffuse and specular but not ambient
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * uObjectColor;

    // Clamp to valid range
    result = clamp(result, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}
