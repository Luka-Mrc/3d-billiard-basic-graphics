#version 330 core

// Input from vertex shader
in vec2 TexCoord;

// Output color
out vec4 FragColor;

// Uniforms
uniform sampler2D uTexture;    // The overlay texture
uniform float uAlpha;          // Overall transparency (0 = invisible, 1 = opaque)

void main()
{
    // Sample the texture
    vec4 texColor = texture(uTexture, TexCoord);

    // Apply overall alpha
    texColor.a *= uAlpha;

    // Discard fully transparent pixels
    if (texColor.a < 0.01)
        discard;

    FragColor = texColor;
}
