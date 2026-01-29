#ifndef TABLE_H
#define TABLE_H

#include "Util.h"
#include "Shader.h"
#include <GL/glew.h>

/**
 * Table Class
 * -----------
 * Represents the billiard table:
 * - Playing surface (flat rectangle at Y = 0)
 * - Four cushions (raised edges around the perimeter)
 *
 * Coordinate System:
 * - Table surface is at Y = 0
 * - Table is centered at origin
 * - Length along Z axis, Width along X axis
 */
class Table
{
public:
    // Table dimensions
    float Width;         // X dimension (shorter side)
    float Length;        // Z dimension (longer side)
    float CushionHeight; // Height of cushions above surface
    float CushionWidth;  // Thickness of cushions

    // Visual properties
    Vec3 SurfaceColor;   // Green felt
    Vec3 CushionColor;   // Cushion wood/rubber color
    Vec3 FrameColor;     // Outer frame color

    /**
     * Constructor with standard pool table dimensions
     */
    Table();

    /**
     * Constructor with custom dimensions
     */
    Table(float width, float length, float cushionHeight, float cushionWidth);

    /**
     * Destructor - cleans up OpenGL resources
     */
    ~Table();

    /**
     * Initialize OpenGL meshes (VAO/VBO/EBO)
     * Must be called after OpenGL context is created
     */
    void InitMesh();

    /**
     * Render the table
     * @param shader Shader to use (should already be bound)
     * @param viewProjection Combined VP matrix
     */
    void Render(Shader& shader, const Mat4& viewProjection);

    // ==================== Table Bounds (for collision detection) ====================

    /**
     * Get the inner playing area bounds
     * @return Vec3 containing half-width, 0, half-length
     */
    Vec3 GetPlayAreaHalfExtents() const;

    /**
     * Get minimum X bound (left cushion inner edge)
     */
    float GetMinX() const;

    /**
     * Get maximum X bound (right cushion inner edge)
     */
    float GetMaxX() const;

    /**
     * Get minimum Z bound (far cushion inner edge)
     */
    float GetMinZ() const;

    /**
     * Get maximum Z bound (near cushion inner edge)
     */
    float GetMaxZ() const;

private:
    // OpenGL objects for surface
    GLuint SurfaceVAO;
    GLuint SurfaceVBO;
    GLuint SurfaceEBO;
    unsigned int SurfaceIndexCount;

    // OpenGL objects for cushions (all 4 combined)
    GLuint CushionVAO;
    GLuint CushionVBO;
    GLuint CushionEBO;
    unsigned int CushionIndexCount;

    // OpenGL objects for frame
    GLuint FrameVAO;
    GLuint FrameVBO;
    GLuint FrameEBO;
    unsigned int FrameIndexCount;

    /**
     * Generate mesh for playing surface
     */
    void GenerateSurfaceMesh();

    /**
     * Generate mesh for cushions
     */
    void GenerateCushionMesh();

    /**
     * Generate mesh for outer frame
     */
    void GenerateFrameMesh();

    /**
     * Helper to upload mesh data to GPU
     */
    void UploadMesh(const MeshData& mesh, GLuint& vao, GLuint& vbo, GLuint& ebo, unsigned int& indexCount);
};

#endif // TABLE_H
