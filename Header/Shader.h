#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>

/**
 * Shader Class
 * ------------
 * Handles loading, compiling, and linking GLSL shaders.
 * Provides uniform setter methods for common data types.
 *
 * Usage:
 *   Shader shader;
 *   shader.Load("vertex.vert", "fragment.frag");
 *   shader.Use();
 *   shader.SetMat4("uMVP", matrixData);
 */
class Shader
{
public:
    // OpenGL program ID
    GLuint ID;

    // Constructor/Destructor
    Shader();
    ~Shader();

    /**
     * Load and compile shaders from file paths
     * @param vertexPath   Path to vertex shader file
     * @param fragmentPath Path to fragment shader file
     * @return true on success, false on failure
     */
    bool Load(const std::string& vertexPath, const std::string& fragmentPath);

    /**
     * Activate this shader program for rendering
     */
    void Use() const;

    // ==================== Uniform Setters ====================

    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;

    // Vector uniforms
    void SetVec2(const std::string& name, float x, float y) const;
    void SetVec3(const std::string& name, float x, float y, float z) const;
    void SetVec3(const std::string& name, const float* value) const;
    void SetVec4(const std::string& name, float x, float y, float z, float w) const;
    void SetVec4(const std::string& name, const float* value) const;

    // Matrix uniform (column-major float[16])
    void SetMat4(const std::string& name, const float* value) const;

private:
    /**
     * Read entire file contents into a string
     */
    std::string ReadFile(const std::string& filePath);

    /**
     * Compile a single shader
     * @param type   GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
     * @param source GLSL source code
     * @return Shader ID, or 0 on failure
     */
    GLuint CompileShader(GLenum type, const std::string& source);

    /**
     * Check and print compilation/linking errors
     */
    void CheckCompileErrors(GLuint shader, const std::string& type);
};

#endif // SHADER_H
