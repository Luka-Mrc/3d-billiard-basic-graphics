#ifndef PHYSICS_H
#define PHYSICS_H

#include "Ball.h"
#include "Table.h"
#include <vector>

/**
 * Physics Constants
 */
namespace PhysicsConstants
{
    // Friction coefficient for rolling on felt
    const float ROLLING_FRICTION = 0.98f;  // Velocity multiplier per second (higher = less friction)

    // Minimum velocity before ball stops completely
    const float MIN_VELOCITY = 0.005f;

    // Coefficient of restitution (bounciness) for ball-ball collisions
    const float BALL_RESTITUTION = 0.95f;

    // Coefficient of restitution for ball-cushion collisions
    const float CUSHION_RESTITUTION = 0.8f;

    // Maximum velocity (speed limit)
    const float MAX_VELOCITY = 10.0f;
}

/**
 * Physics Engine
 * --------------
 * Handles all physics simulation:
 * - Ball movement integration
 * - Ball-ball collision detection and response
 * - Ball-cushion collision detection and response
 * - Rolling friction
 *
 * No spin or angular momentum (simplified model)
 */
class Physics
{
public:
    /**
     * Constructor
     */
    Physics();

    /**
     * Update physics for all balls
     * @param balls Vector of ball pointers
     * @param table Reference to the table
     * @param deltaTime Time step in seconds
     */
    void Update(std::vector<Ball*>& balls, const Table& table, float deltaTime);

    /**
     * Apply an impulse to a ball (e.g., cue strike)
     * @param ball Ball to hit
     * @param direction Direction of impulse (will be normalized)
     * @param power Strength of impulse
     */
    void ApplyImpulse(Ball* ball, const Vec3& direction, float power);

    /**
     * Check if all balls have stopped moving
     */
    bool AllBallsStopped(const std::vector<Ball*>& balls) const;

private:
    /**
     * Apply friction to slow down balls
     */
    void ApplyFriction(std::vector<Ball*>& balls, float deltaTime);

    /**
     * Integrate ball positions based on velocities
     */
    void IntegratePositions(std::vector<Ball*>& balls, float deltaTime);

    /**
     * Detect and resolve ball-ball collisions
     */
    void ResolveBallCollisions(std::vector<Ball*>& balls);

    /**
     * Detect and resolve ball-cushion collisions
     */
    void ResolveCushionCollisions(std::vector<Ball*>& balls, const Table& table);

    /**
     * Check collision between two balls
     * @return true if balls are overlapping
     */
    bool CheckBallCollision(const Ball* a, const Ball* b) const;

    /**
     * Resolve collision between two balls
     * Updates velocities based on elastic collision
     */
    void ResolveBallCollision(Ball* a, Ball* b);

    /**
     * Clamp ball velocities to maximum
     */
    void ClampVelocities(std::vector<Ball*>& balls);

    /**
     * Stop balls that are moving very slowly
     */
    void StopSlowBalls(std::vector<Ball*>& balls);
};

#endif // PHYSICS_H
