#define _CRT_SECURE_NO_WARNINGS

#include "../Header/Util.h"
#include <iostream>
#include <cstring>

// ============================================================================
// STB_IMAGE IMPLEMENTATION
// Single-header image loading library
// ============================================================================
#define STB_IMAGE_IMPLEMENTATION
#include "../Header/stb_image.h"

// ============================================================================
// MAT4 IMPLEMENTATION
// ============================================================================

Mat4 Mat4::operator*(const Mat4& other) const
{
    Mat4 result;
    // Clear result to zero
    for (int i = 0; i < 16; i++) result.m[i] = 0.0f;

    // Matrix multiplication (column-major)
    for (int col = 0; col < 4; col++)
    {
        for (int row = 0; row < 4; row++)
        {
            for (int k = 0; k < 4; k++)
            {
                result.m[col * 4 + row] += m[k * 4 + row] * other.m[col * 4 + k];
            }
        }
    }
    return result;
}

Mat4 Mat4::Identity()
{
    return Mat4(); // Default constructor creates identity
}

Mat4 Mat4::Translate(float x, float y, float z)
{
    Mat4 result;
    result.m[12] = x;
    result.m[13] = y;
    result.m[14] = z;
    return result;
}

Mat4 Mat4::Translate(const Vec3& v)
{
    return Translate(v.x, v.y, v.z);
}

Mat4 Mat4::Scale(float x, float y, float z)
{
    Mat4 result;
    result.m[0] = x;
    result.m[5] = y;
    result.m[10] = z;
    return result;
}

Mat4 Mat4::Scale(float s)
{
    return Scale(s, s, s);
}

Mat4 Mat4::RotateX(float radians)
{
    Mat4 result;
    float c = cosf(radians);
    float s = sinf(radians);
    result.m[5] = c;
    result.m[6] = s;
    result.m[9] = -s;
    result.m[10] = c;
    return result;
}

Mat4 Mat4::RotateY(float radians)
{
    Mat4 result;
    float c = cosf(radians);
    float s = sinf(radians);
    result.m[0] = c;
    result.m[2] = -s;
    result.m[8] = s;
    result.m[10] = c;
    return result;
}

Mat4 Mat4::RotateZ(float radians)
{
    Mat4 result;
    float c = cosf(radians);
    float s = sinf(radians);
    result.m[0] = c;
    result.m[1] = s;
    result.m[4] = -s;
    result.m[5] = c;
    return result;
}

Mat4 Mat4::Perspective(float fovY, float aspect, float nearPlane, float farPlane)
{
    Mat4 result;
    // Clear to zero
    for (int i = 0; i < 16; i++) result.m[i] = 0.0f;

    float tanHalfFov = tanf(fovY / 2.0f);

    result.m[0] = 1.0f / (aspect * tanHalfFov);
    result.m[5] = 1.0f / tanHalfFov;
    result.m[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result.m[11] = -1.0f;
    result.m[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    result.m[15] = 0.0f;

    return result;
}

Mat4 Mat4::Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    Mat4 result;
    for (int i = 0; i < 16; i++) result.m[i] = 0.0f;

    result.m[0]  =  2.0f / (right - left);
    result.m[5]  =  2.0f / (top - bottom);
    result.m[10] = -2.0f / (farPlane - nearPlane);
    result.m[12] = -(right + left) / (right - left);
    result.m[13] = -(top + bottom) / (top - bottom);
    result.m[14] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result.m[15] =  1.0f;

    return result;
}

Mat4 Mat4::LookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
{
    Vec3 f = (target - eye).Normalized();  // Forward
    Vec3 r = Cross(f, up).Normalized();     // Right
    Vec3 u = Cross(r, f);                   // Up (recalculated)

    Mat4 result;

    // Rotation part
    result.m[0] = r.x;
    result.m[1] = u.x;
    result.m[2] = -f.x;
    result.m[3] = 0.0f;

    result.m[4] = r.y;
    result.m[5] = u.y;
    result.m[6] = -f.y;
    result.m[7] = 0.0f;

    result.m[8] = r.z;
    result.m[9] = u.z;
    result.m[10] = -f.z;
    result.m[11] = 0.0f;

    // Translation part
    result.m[12] = -Dot(r, eye);
    result.m[13] = -Dot(u, eye);
    result.m[14] = Dot(f, eye);
    result.m[15] = 1.0f;

    return result;
}

Mat4 Mat4::Inverse() const
{
    float inv[16], det;

    inv[0] = m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] = m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];

    inv[1] = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] = m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] = -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] = m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];

    inv[2] = m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] = -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] = m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];

    inv[3] = -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] = m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11] - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] = m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10] + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];

    if (fabsf(det) < 0.00001f)
        return Mat4::Identity();

    det = 1.0f / det;

    Mat4 result;
    for (int i = 0; i < 16; i++)
        result.m[i] = inv[i] * det;

    return result;
}

// ============================================================================
// MESH GENERATION
// ============================================================================

MeshData GenerateSphereMesh(float radius, int sectors, int stacks)
{
    MeshData mesh;

    float sectorStep = 2.0f * PI / sectors;
    float stackStep = PI / stacks;

    // Generate vertices
    for (int i = 0; i <= stacks; ++i)
    {
        float stackAngle = PI / 2.0f - i * stackStep;  // From pi/2 to -pi/2
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j)
        {
            float sectorAngle = j * sectorStep;

            Vertex v;

            // Position
            v.position[0] = xy * cosf(sectorAngle);
            v.position[1] = z;  // Y is up
            v.position[2] = xy * sinf(sectorAngle);

            // Normal (normalized position for unit sphere, scaled)
            float nx = v.position[0] / radius;
            float ny = v.position[1] / radius;
            float nz = v.position[2] / radius;
            v.normal[0] = nx;
            v.normal[1] = ny;
            v.normal[2] = nz;

            // Texture coordinates
            v.texCoord[0] = (float)j / sectors;
            v.texCoord[1] = (float)i / stacks;

            mesh.vertices.push_back(v);
        }
    }

    // Generate indices
    for (int i = 0; i < stacks; ++i)
    {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            // Two triangles per sector (except at poles)
            if (i != 0)
            {
                mesh.indices.push_back(k1);
                mesh.indices.push_back(k2);
                mesh.indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1))
            {
                mesh.indices.push_back(k1 + 1);
                mesh.indices.push_back(k2);
                mesh.indices.push_back(k2 + 1);
            }
        }
    }

    return mesh;
}

MeshData GenerateQuadMesh()
{
    MeshData mesh;

    // Quad from (0,0) to (1,1) in XY plane
    // Counter-clockwise winding

    Vertex v0, v1, v2, v3;

    // Bottom-left
    v0.position[0] = 0.0f; v0.position[1] = 0.0f; v0.position[2] = 0.0f;
    v0.normal[0] = 0.0f;   v0.normal[1] = 0.0f;   v0.normal[2] = 1.0f;
    v0.texCoord[0] = 0.0f; v0.texCoord[1] = 0.0f;

    // Bottom-right
    v1.position[0] = 1.0f; v1.position[1] = 0.0f; v1.position[2] = 0.0f;
    v1.normal[0] = 0.0f;   v1.normal[1] = 0.0f;   v1.normal[2] = 1.0f;
    v1.texCoord[0] = 1.0f; v1.texCoord[1] = 0.0f;

    // Top-right
    v2.position[0] = 1.0f; v2.position[1] = 1.0f; v2.position[2] = 0.0f;
    v2.normal[0] = 0.0f;   v2.normal[1] = 0.0f;   v2.normal[2] = 1.0f;
    v2.texCoord[0] = 1.0f; v2.texCoord[1] = 1.0f;

    // Top-left
    v3.position[0] = 0.0f; v3.position[1] = 1.0f; v3.position[2] = 0.0f;
    v3.normal[0] = 0.0f;   v3.normal[1] = 0.0f;   v3.normal[2] = 1.0f;
    v3.texCoord[0] = 0.0f; v3.texCoord[1] = 1.0f;

    mesh.vertices.push_back(v0);
    mesh.vertices.push_back(v1);
    mesh.vertices.push_back(v2);
    mesh.vertices.push_back(v3);

    // Two triangles
    mesh.indices.push_back(0);
    mesh.indices.push_back(1);
    mesh.indices.push_back(2);

    mesh.indices.push_back(0);
    mesh.indices.push_back(2);
    mesh.indices.push_back(3);

    return mesh;
}

MeshData GenerateBoxMesh(float width, float height, float depth)
{
    MeshData mesh;

    float hw = width / 2.0f;
    float hh = height / 2.0f;
    float hd = depth / 2.0f;

    // Helper to add a face (4 vertices, 6 indices for 2 triangles)
    auto addFace = [&mesh](
        float p0x, float p0y, float p0z,
        float p1x, float p1y, float p1z,
        float p2x, float p2y, float p2z,
        float p3x, float p3y, float p3z,
        float nx, float ny, float nz)
    {
        unsigned int baseIndex = (unsigned int)mesh.vertices.size();

        Vertex v0, v1, v2, v3;

        v0.position[0] = p0x; v0.position[1] = p0y; v0.position[2] = p0z;
        v0.normal[0] = nx;    v0.normal[1] = ny;    v0.normal[2] = nz;
        v0.texCoord[0] = 0.0f; v0.texCoord[1] = 0.0f;

        v1.position[0] = p1x; v1.position[1] = p1y; v1.position[2] = p1z;
        v1.normal[0] = nx;    v1.normal[1] = ny;    v1.normal[2] = nz;
        v1.texCoord[0] = 1.0f; v1.texCoord[1] = 0.0f;

        v2.position[0] = p2x; v2.position[1] = p2y; v2.position[2] = p2z;
        v2.normal[0] = nx;    v2.normal[1] = ny;    v2.normal[2] = nz;
        v2.texCoord[0] = 1.0f; v2.texCoord[1] = 1.0f;

        v3.position[0] = p3x; v3.position[1] = p3y; v3.position[2] = p3z;
        v3.normal[0] = nx;    v3.normal[1] = ny;    v3.normal[2] = nz;
        v3.texCoord[0] = 0.0f; v3.texCoord[1] = 1.0f;

        mesh.vertices.push_back(v0);
        mesh.vertices.push_back(v1);
        mesh.vertices.push_back(v2);
        mesh.vertices.push_back(v3);

        // Two triangles (CCW)
        mesh.indices.push_back(baseIndex + 0);
        mesh.indices.push_back(baseIndex + 1);
        mesh.indices.push_back(baseIndex + 2);

        mesh.indices.push_back(baseIndex + 0);
        mesh.indices.push_back(baseIndex + 2);
        mesh.indices.push_back(baseIndex + 3);
    };

    // Front face (+Z)
    addFace(-hw, -hh, hd,  hw, -hh, hd,  hw, hh, hd,  -hw, hh, hd,  0, 0, 1);

    // Back face (-Z)
    addFace(hw, -hh, -hd,  -hw, -hh, -hd,  -hw, hh, -hd,  hw, hh, -hd,  0, 0, -1);

    // Right face (+X)
    addFace(hw, -hh, hd,  hw, -hh, -hd,  hw, hh, -hd,  hw, hh, hd,  1, 0, 0);

    // Left face (-X)
    addFace(-hw, -hh, -hd,  -hw, -hh, hd,  -hw, hh, hd,  -hw, hh, -hd,  -1, 0, 0);

    // Top face (+Y)
    addFace(-hw, hh, hd,  hw, hh, hd,  hw, hh, -hd,  -hw, hh, -hd,  0, 1, 0);

    // Bottom face (-Y)
    addFace(-hw, -hh, -hd,  hw, -hh, -hd,  hw, -hh, hd,  -hw, -hh, hd,  0, -1, 0);

    return mesh;
}

MeshData GenerateDiscMesh(float radius, int segments)
{
    MeshData mesh;

    // Center vertex
    Vertex center;
    center.position[0] = 0.0f;
    center.position[1] = 0.0f;
    center.position[2] = 0.0f;
    center.normal[0] = 0.0f;
    center.normal[1] = 1.0f;
    center.normal[2] = 0.0f;
    center.texCoord[0] = 0.5f;
    center.texCoord[1] = 0.5f;
    mesh.vertices.push_back(center);

    // Perimeter vertices
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * PI * i / segments;
        Vertex v;
        v.position[0] = radius * cosf(angle);
        v.position[1] = 0.0f;
        v.position[2] = radius * sinf(angle);
        v.normal[0] = 0.0f;
        v.normal[1] = 1.0f;
        v.normal[2] = 0.0f;
        v.texCoord[0] = 0.5f + 0.5f * cosf(angle);
        v.texCoord[1] = 0.5f + 0.5f * sinf(angle);
        mesh.vertices.push_back(v);
    }

    // Triangle fan indices (wound CCW when viewed from +Y)
    for (int i = 1; i <= segments; i++)
    {
        mesh.indices.push_back(0);
        mesh.indices.push_back(i + 1);
        mesh.indices.push_back(i);
    }

    return mesh;
}

// ============================================================================
// TEXTURE LOADING
// ============================================================================

GLuint LoadTexture(const char* filePath, bool flipY)
{
    // Configure stb_image to flip vertically if requested
    stbi_set_flip_vertically_on_load(flipY ? 1 : 0);

    int width, height, channels;
    unsigned char* data = stbi_load(filePath, &width, &height, &channels, 0);

    if (!data)
    {
        std::cerr << "ERROR::TEXTURE::FAILED_TO_LOAD: " << filePath << std::endl;
        return 0;
    }

    // Determine format based on channels
    GLenum format = GL_RGB;
    GLenum internalFormat = GL_RGB;
    if (channels == 1)
    {
        format = GL_RED;
        internalFormat = GL_RED;
    }
    else if (channels == 3)
    {
        format = GL_RGB;
        internalFormat = GL_RGB;
    }
    else if (channels == 4)
    {
        format = GL_RGBA;
        internalFormat = GL_RGBA;
    }

    // Generate and configure texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Free image data
    stbi_image_free(data);

    std::cout << "Loaded texture: " << filePath << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;

    return textureID;
}
