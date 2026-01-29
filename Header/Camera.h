#ifndef CAMERA_H
#define CAMERA_H

#include "Util.h"

/**
 * Camera Class
 * ------------
 * Manages view and projection matrices for 3D rendering.
 * Positioned above the billiard table looking down.
 *
 * Coordinate System:
 * - Y is up
 * - Table surface is at Y = 0
 * - Camera looks down at the table from above
 */
class Camera
{
public:
    // Camera position in world space
    Vec3 Position;

    // Point the camera is looking at
    Vec3 Target;

    // Up direction (usually Y-up)
    Vec3 Up;

    // Field of view in radians
    float FOV;

    // Aspect ratio (width / height)
    float AspectRatio;

    // Near and far clipping planes
    float NearPlane;
    float FarPlane;

    /**
     * Constructor with default values for top-down billiards view
     */
    Camera();

    /**
     * Set camera position
     */
    void SetPosition(float x, float y, float z);
    void SetPosition(const Vec3& pos);

    /**
     * Set target point
     */
    void SetTarget(float x, float y, float z);
    void SetTarget(const Vec3& target);

    /**
     * Set perspective parameters
     */
    void SetPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane);

    /**
     * Update aspect ratio (call when window resizes)
     */
    void SetAspectRatio(float aspect);

    /**
     * Get the view matrix (world -> camera space)
     */
    Mat4 GetViewMatrix() const;

    /**
     * Get the projection matrix (camera -> clip space)
     */
    Mat4 GetProjectionMatrix() const;

    /**
     * Get combined view-projection matrix
     */
    Mat4 GetViewProjectionMatrix() const;

    /**
     * Get the camera's forward direction
     */
    Vec3 GetForward() const;

    /**
     * Get the camera's right direction
     */
    Vec3 GetRight() const;
};

#endif // CAMERA_H
