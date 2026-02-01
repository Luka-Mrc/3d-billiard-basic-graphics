#version 330 core
// Light source (spotlight from above)
struct Light {
    vec3 pos;           // Position
    vec3 dir;           // Spotlight direction (normalized, pointing down)
    vec3 kA;            // Ambient component (indirect light)
    vec3 kD;            // Diffuse component (direct light)
    vec3 kS;            // Specular component (highlight)
    float cutOff;       // cos(inner cone angle)
    float outerCutOff;  // cos(outer cone angle)
};

// Material properties
struct Material {
    vec3 kS;            // Specular reflectance color
    float shine;        // Shininess (glossiness)
};

// Input from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

// Output color
out vec4 FragColor;

// Uniforms
uniform Light uLight;
uniform Material uMaterial;
uniform vec3 uObjectColor;    // Base color (used as ambient & diffuse reflectance)
uniform vec3 uViewPos;        // Camera position (for specular calculation)
uniform sampler2D uShadowMap; // Shadow depth map
uniform mat4 uLightSpaceMatrix;

// ============================================================================
// Shadow calculation with PCF 5x5
// ============================================================================
float ShadowCalculation(vec4 fragPosLS, vec3 normal, vec3 lightDir)
{
    // Perspective divide
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    // Transform from [-1,1] to [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    // Fragment is outside the shadow map
    if (projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;

    // Bias based on surface angle to light (prevents shadow acne)
    float bias = max(0.003 * (1.0 - dot(normal, lightDir)), 0.001);

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

// ============================================================================
// Main - Phong shading model with spotlight
// ============================================================================
void main()
{
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(uLight.pos - FragPos);
    vec3 viewDir = normalize(uViewPos - FragPos);

    // === Spotlight cone intensity ===
    // theta = angle between light-to-fragment direction and spotlight direction
    float theta = dot(lightDir, normalize(-uLight.dir));
    float epsilon = uLight.cutOff - uLight.outerCutOff;
    float spotIntensity = clamp((theta - uLight.outerCutOff) / epsilon, 0.0, 1.0);

    // === Distance attenuation ===
    float dist = length(uLight.pos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.007 * dist + 0.002 * dist * dist);

    // === Ambient (Phong model - not affected by spotlight or shadow) ===
    vec3 resA = uLight.kA * uObjectColor;

    // === Diffuse (Phong model) ===
    float nD = max(dot(normal, lightDir), 0.0);
    vec3 resD = uLight.kD * (nD * uObjectColor);

    // === Specular (Phong reflection model) ===
    vec3 reflectDir = reflect(-lightDir, normal);
    float s = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shine);
    vec3 resS = uLight.kS * (s * uMaterial.kS);

    // Apply attenuation and spotlight to diffuse and specular only
    resD *= attenuation * spotIntensity;
    resS *= attenuation * spotIntensity;

    // === Shadow ===
    float shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);

    // Final color: ambient always visible, diffuse+specular affected by shadow and spotlight
    vec3 result = resA + (1.0 - shadow) * (resD + resS);

    result = clamp(result, 0.0, 1.0);
    FragColor = vec4(result, 1.0);
}
