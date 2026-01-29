#version 330 core

// Input vertex attributes
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;    // Not used for 2D overlay, but keeping for consistent layout
layout (location = 2) in vec2 aTexCoord;

// Output to fragment shader
out vec2 TexCoord;

// Uniforms for positioning the quad in screen space
uniform vec2 uPosition;    // Position in normalized device coordinates (-1 to 1)
uniform vec2 uSize;        // Size in normalized device coordinates

void main()
{
    // The input quad goes from (0,0) to (1,1)
    // Transform to NDC position and size
    vec2 pos = aPosition.xy * uSize + uPosition;

    // Z = 0 for 2D overlay, W = 1
    gl_Position = vec4(pos, 0.0, 1.0);

    // Pass texture coordinates
    TexCoord = aTexCoord;
}
