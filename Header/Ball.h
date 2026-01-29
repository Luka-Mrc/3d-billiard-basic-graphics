#ifndef BALL_H
#define BALL_H

#include "Util.h"
#include "Shader.h"
#include <GL/glew.h>

/**
 * Ball Class
 * ----------
 * Represents a single billiard ball with:
 * - Position (center of sphere, Y = radius when on table surface)
 * - Velocity (movement direction and speed)
 * - Radius
 * - Color
 * - OpenGL mesh (VAO/VBO/EBO)
 *
 * Balls move on the XZ plane with Y = radius (resting on table surface at Y=0)
 */
class Ball
{
public:
    // Physics properties
    Vec3 Position;       // Center of the ball
    Vec3 Velocity;       // Current velocity
    float Radius;        // Ball radius

    // Visual properties
    Vec3 Color;          // RGB color (0-1 range)

    // State
    bool IsActive;       // False if ball is pocketed/removed

    // Ball number (for identification)
    int Number;

    /**
     * Constructor
     * @param number Ball number (0 = cue ball, 1-15 = colored balls)
     * @param position Initial position
     * @param radius Ball radius
     * @param color Ball color (RGB)
     */
    Ball(int number, const Vec3& position, float radius, const Vec3& color);

    /**
     * Destructor - cleans up OpenGL resources
     */
    ~Ball();

    /**
     * Initialize OpenGL mesh (VAO/VBO/EBO)
     * Must be called after OpenGL context is created
     */
    void InitMesh();

    /**
     * Render the ball
     * @param shader Shader to use (should already be bound)
     * @param viewProjection Combined VP matrix
     */
    void Render(Shader& shader, const Mat4& viewProjection);

    /**
     * Update ball position based on velocity
     * @param deltaTime Time step in seconds
     */
    void Update(float deltaTime);

    /**
     * Get the model matrix for this ball
     */
    Mat4 GetModelMatrix() const;

    /**
     * Check if ball is moving (velocity > threshold)
     */
    bool IsMoving() const;

    /**
     * Stop the ball (set velocity to zero)
     */
    void Stop();

private:
    // OpenGL objects
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    unsigned int IndexCount;

    // Mesh generation parameters
    static const int SPHERE_SECTORS = 32;
    static const int SPHERE_STACKS = 16;

    // Velocity threshold for considering ball "stopped"
    static constexpr float VELOCITY_THRESHOLD = 0.001f;
};

/**
 * Create a standard set of billiard balls
 * Ball 0 = Cue ball (white)
 * Balls 1-7 = Solids
 * Ball 8 = 8-ball (black)
 * Balls 9-15 = Stripes (simplified as solid colors)
 *
 * @param ballRadius Radius for all balls
 * @return Vector of Ball pointers
 */
std::vector<Ball*> CreateStandardBallSet(float ballRadius);

#endif // BALL_H
