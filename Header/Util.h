#ifndef UTIL_H
#define UTIL_H

#include <GL/glew.h>
#include <vector>
#include <cmath>

// ============================================================================
// MATH TYPES - Simple GLM-style vector and matrix types
// ============================================================================

/**
 * 3D Vector
 */
struct Vec3
{
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Pointer to data (for OpenGL uniforms)
    const float* Ptr() const { return &x; }

    // Basic operations
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }

    Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

    float Length() const { return sqrtf(x * x + y * y + z * z); }
    float LengthSquared() const { return x * x + y * y + z * z; }

    Vec3 Normalized() const
    {
        float len = Length();
        if (len > 0.0001f)
            return *this / len;
        return Vec3(0, 0, 0);
    }

    void Normalize()
    {
        float len = Length();
        if (len > 0.0001f)
        {
            x /= len;
            y /= len;
            z /= len;
        }
    }
};

// Vector operations
inline float Dot(const Vec3& a, const Vec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 Cross(const Vec3& a, const Vec3& b)
{
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

inline Vec3 operator*(float s, const Vec3& v)
{
    return Vec3(s * v.x, s * v.y, s * v.z);
}

/**
 * 4x4 Matrix (column-major order for OpenGL)
 */
struct Mat4
{
    float m[16];

    Mat4()
    {
        // Initialize to identity
        for (int i = 0; i < 16; i++) m[i] = 0.0f;
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    // Pointer to data (for OpenGL uniforms)
    const float* Ptr() const { return m; }

    // Access element at [row][col]
    float& At(int row, int col) { return m[col * 4 + row]; }
    float At(int row, int col) const { return m[col * 4 + row]; }

    // Matrix multiplication
    Mat4 operator*(const Mat4& other) const;

    // Static constructors
    static Mat4 Identity();
    static Mat4 Translate(float x, float y, float z);
    static Mat4 Translate(const Vec3& v);
    static Mat4 Scale(float x, float y, float z);
    static Mat4 Scale(float s);
    static Mat4 RotateX(float radians);
    static Mat4 RotateY(float radians);
    static Mat4 RotateZ(float radians);
    static Mat4 Perspective(float fovY, float aspect, float nearPlane, float farPlane);
    static Mat4 Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    static Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up);

    Mat4 Inverse() const;
};

// ============================================================================
// MESH DATA STRUCTURE
// ============================================================================

/**
 * Vertex with position, normal, and texture coordinates
 */
struct Vertex
{
    float position[3];
    float normal[3];
    float texCoord[2];
};

/**
 * Mesh data container
 */
struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

// ============================================================================
// MESH GENERATION FUNCTIONS
// ============================================================================

/**
 * Generate a UV sphere mesh
 * @param radius      Sphere radius
 * @param sectors     Horizontal divisions (longitude)
 * @param stacks      Vertical divisions (latitude)
 * @return MeshData with vertices and indices
 */
MeshData GenerateSphereMesh(float radius, int sectors = 32, int stacks = 16);

/**
 * Generate a simple quad mesh for 2D overlay
 * Quad spans from (x, y) to (x+w, y+h) in screen coordinates
 * @return MeshData for a unit quad (0,0) to (1,1) - scale/translate as needed
 */
MeshData GenerateQuadMesh();

/**
 * Generate a box/cuboid mesh
 * @param width  Size along X axis
 * @param height Size along Y axis
 * @param depth  Size along Z axis
 * @return MeshData centered at origin
 */
MeshData GenerateBoxMesh(float width, float height, float depth);

/**
 * Generate a flat disc (circle) mesh on the XZ plane
 * @param radius    Disc radius
 * @param segments  Number of segments around the circle
 * @return MeshData centered at origin, lying on Y=0
 */
MeshData GenerateDiscMesh(float radius, int segments = 32);

// ============================================================================
// TEXTURE LOADING
// ============================================================================

/**
 * Load a texture from file (PNG, JPG, etc.)
 * Uses stb_image internally
 * @param filePath Path to image file
 * @param flipY    Flip image vertically (OpenGL expects bottom-left origin)
 * @return OpenGL texture ID, or 0 on failure
 */
GLuint LoadTexture(const char* filePath, bool flipY = true);

// ============================================================================
// UTILITY CONSTANTS
// ============================================================================

const float PI = 3.14159265358979323846f;
const float DEG_TO_RAD = PI / 180.0f;
const float RAD_TO_DEG = 180.0f / PI;

inline float Radians(float degrees) { return degrees * DEG_TO_RAD; }
inline float Degrees(float radians) { return radians * RAD_TO_DEG; }

// Clamp value between min and max
inline float Clamp(float value, float minVal, float maxVal)
{
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

#endif // UTIL_H
