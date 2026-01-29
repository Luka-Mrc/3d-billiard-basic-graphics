#include "../Header/Camera.h"

Camera::Camera()
    : Position(0.0f, 5.0f, 8.0f)   // Elevated position behind the table
    , Target(0.0f, 0.0f, 0.0f)     // Looking at table center
    , Up(0.0f, 1.0f, 0.0f)         // Y is up
    , FOV(Radians(45.0f))          // 45 degree field of view
    , AspectRatio(16.0f / 9.0f)    // Default 16:9
    , NearPlane(0.1f)
    , FarPlane(100.0f)
{
}

void Camera::SetPosition(float x, float y, float z)
{
    Position = Vec3(x, y, z);
}

void Camera::SetPosition(const Vec3& pos)
{
    Position = pos;
}

void Camera::SetTarget(float x, float y, float z)
{
    Target = Vec3(x, y, z);
}

void Camera::SetTarget(const Vec3& target)
{
    Target = target;
}

void Camera::SetPerspective(float fovDegrees, float aspect, float nearPlane, float farPlane)
{
    FOV = Radians(fovDegrees);
    AspectRatio = aspect;
    NearPlane = nearPlane;
    FarPlane = farPlane;
}

void Camera::SetAspectRatio(float aspect)
{
    AspectRatio = aspect;
}

Mat4 Camera::GetViewMatrix() const
{
    return Mat4::LookAt(Position, Target, Up);
}

Mat4 Camera::GetProjectionMatrix() const
{
    return Mat4::Perspective(FOV, AspectRatio, NearPlane, FarPlane);
}

Mat4 Camera::GetViewProjectionMatrix() const
{
    return GetProjectionMatrix() * GetViewMatrix();
}

Vec3 Camera::GetForward() const
{
    return (Target - Position).Normalized();
}

Vec3 Camera::GetRight() const
{
    return Cross(GetForward(), Up).Normalized();
}
