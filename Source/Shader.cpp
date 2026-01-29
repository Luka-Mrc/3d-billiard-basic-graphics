#include "../Header/Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader() : ID(0)
{
}

Shader::~Shader()
{
    if (ID != 0)
    {
        glDeleteProgram(ID);
    }
}

bool Shader::Load(const std::string& vertexPath, const std::string& fragmentPath)
{
    // Read shader source files
    std::string vertexCode = ReadFile(vertexPath);
    std::string fragmentCode = ReadFile(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty())
    {
        std::cerr << "ERROR::SHADER::FILE_READ_FAILED" << std::endl;
        return false;
    }

    // Compile shaders
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexCode);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentCode);

    if (vertexShader == 0 || fragmentShader == 0)
    {
        return false;
    }

    // Link program
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    CheckCompileErrors(ID, "PROGRAM");

    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

void Shader::Use() const
{
    glUseProgram(ID);
}

// ==================== Uniform Setters ====================

void Shader::SetBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::SetInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::SetVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::SetVec3(const std::string& name, const float* value) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value);
}

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::SetVec4(const std::string& name, const float* value) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, value);
}

void Shader::SetMat4(const std::string& name, const float* value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
}

// ==================== Private Helpers ====================

std::string Shader::ReadFile(const std::string& filePath)
{
    std::ifstream file;
    std::stringstream stream;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        file.open(filePath);
        stream << file.rdbuf();
        file.close();
        return stream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filePath << std::endl;
        return "";
    }
}

GLuint Shader::CompileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    std::string typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    CheckCompileErrors(shader, typeStr);

    // Check if compilation succeeded
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

void Shader::CheckCompileErrors(GLuint shader, const std::string& type)
{
    GLint success;
    GLchar infoLog[1024];

    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- "
                      << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- "
                      << std::endl;
        }
    }
}
