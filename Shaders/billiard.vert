#version 330 core

// Input vertex attributes
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// Output to fragment shader
out vec3 FragPos;              // Fragment position in world space
out vec3 Normal;               // Normal in world space
out vec2 TexCoord;             // Texture coordinates
out vec4 FragPosLightSpace;    // Fragment position in light clip space

// Uniforms
uniform mat4 uMVP;               // Model-View-Projection matrix
uniform mat4 uModel;             // Model matrix (for world space calculations)
uniform mat4 uLightSpaceMatrix;  // Light view-projection matrix

void main()
{
    // Transform vertex position to clip space
    gl_Position = uMVP * vec4(aPosition, 1.0);

    // Calculate fragment position in world space
    FragPos = vec3(uModel * vec4(aPosition, 1.0));

    // Transform normal to world space using the normal matrix
    // transpose(inverse(uModel)) handles non-uniform scaling correctly
    Normal = mat3(transpose(inverse(uModel))) * aNormal;

    // Pass through texture coordinates
    TexCoord = aTexCoord;

    // Transform world position to light clip space for shadow mapping
    FragPosLightSpace = uLightSpaceMatrix * vec4(FragPos, 1.0);
}
