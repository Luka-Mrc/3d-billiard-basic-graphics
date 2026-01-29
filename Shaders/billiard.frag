#version 330 core

// Input from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Output color
out vec4 FragColor;

// Uniforms
uniform vec3 uObjectColor;    // Base color of the object
uniform vec3 uLightDir;       // Direction TO the light (normalized)
uniform vec3 uLightColor;     // Light color (usually white)
uniform vec3 uViewPos;        // Camera position for specular
uniform float uAmbient;       // Ambient light intensity
uniform float uSpecular;      // Specular intensity
uniform float uShininess;     // Specular shininess exponent

void main()
{
    // Normalize the normal (may have been interpolated)
    vec3 norm = normalize(Normal);

    // ============ Ambient ============
    vec3 ambient = uAmbient * uLightColor;

    // ============ Diffuse ============
    // Light direction is already pointing toward light
    float diff = max(dot(norm, uLightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    // ============ Specular (Blinn-Phong) ============
    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 halfDir = normalize(uLightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), uShininess);
    vec3 specular = uSpecular * spec * uLightColor;

    // ============ Combine ============
    vec3 result = (ambient + diffuse + specular) * uObjectColor;

    // Clamp to valid range
    result = clamp(result, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}
