#version 330 core

// Input vertex attributes
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// Output to fragment shader
out vec3 FragPos;      // Fragment position in world space
out vec3 Normal;       // Normal in world space
out vec2 TexCoord;     // Texture coordinates

// Uniforms
uniform mat4 uMVP;     // Model-View-Projection matrix
uniform mat4 uModel;   // Model matrix (for world space calculations)

void main()
{
    // Transform vertex position to clip space
    gl_Position = uMVP * vec4(aPosition, 1.0);

    // Calculate fragment position in world space
    FragPos = vec3(uModel * vec4(aPosition, 1.0));

    // Transform normal to world space
    // Using mat3 to remove translation, and transpose(inverse()) for non-uniform scaling
    // Simplified: assuming uniform scaling, we just use mat3(uModel)
    Normal = mat3(uModel) * aNormal;

    // Pass through texture coordinates
    TexCoord = aTexCoord;
}
