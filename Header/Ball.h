#ifndef BALL_H
#define BALL_H

#include "Util.h"
#include "Shader.h"
#include "Model.h"
#include <GL/glew.h>

class Ball
{
public:
    // Physics properties
    Vec3 Position;
    Vec3 Velocity;
    float Radius;

    // Visual properties
    Vec3 Color;

    // State
    bool IsActive;

    // Ball number (for identification)
    int Number;

    Ball(int number, const Vec3& position, float radius, const Vec3& color);
    ~Ball();

    /**
     * Load the shared sphere model from file (call once at startup)
     * @param path Path to the sphere .obj model file
     */
    static void LoadModel(const std::string& path);

    /**
     * Cleanup the shared model (call once at shutdown)
     */
    static void CleanupModel();

    void Render(Shader& shader, const Mat4& viewProjection);
    void Update(float deltaTime);
    Mat4 GetModelMatrix() const;
    bool IsMoving() const;
    void Stop();

private:
    // Shared 3D model for all balls
    static Model* s_SphereModel;

    // Velocity threshold for considering ball "stopped"
    static constexpr float VELOCITY_THRESHOLD = 0.001f;
};

std::vector<Ball*> CreateStandardBallSet(float ballRadius);

#endif // BALL_H
