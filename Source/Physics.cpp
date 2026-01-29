#include "../Header/Physics.h"
#include <cmath>

Physics::Physics()
{
}

void Physics::Update(std::vector<Ball*>& balls, const Table& table, float deltaTime)
{
    // Apply friction first
    ApplyFriction(balls, deltaTime);

    // Integrate positions
    IntegratePositions(balls, deltaTime);

    // Resolve collisions (multiple iterations for stability)
    const int COLLISION_ITERATIONS = 3;
    for (int i = 0; i < COLLISION_ITERATIONS; i++)
    {
        ResolveBallCollisions(balls);
        ResolveCushionCollisions(balls, table);
    }

    // Clamp velocities and stop slow balls
    ClampVelocities(balls);
    StopSlowBalls(balls);
}

void Physics::ApplyImpulse(Ball* ball, const Vec3& direction, float power)
{
    if (!ball || !ball->IsActive)
        return;

    Vec3 impulse = direction.Normalized() * power;
    ball->Velocity += impulse;

    // Clamp to max velocity
    float speed = ball->Velocity.Length();
    if (speed > PhysicsConstants::MAX_VELOCITY)
    {
        ball->Velocity = ball->Velocity.Normalized() * PhysicsConstants::MAX_VELOCITY;
    }
}

bool Physics::AllBallsStopped(const std::vector<Ball*>& balls) const
{
    for (const Ball* ball : balls)
    {
        if (ball->IsActive && ball->IsMoving())
            return false;
    }
    return true;
}

void Physics::ApplyFriction(std::vector<Ball*>& balls, float deltaTime)
{
    // Friction factor per frame
    // We want ROLLING_FRICTION to be the factor per second
    // So per frame it's ROLLING_FRICTION^deltaTime
    float frictionFactor = powf(PhysicsConstants::ROLLING_FRICTION, deltaTime);

    for (Ball* ball : balls)
    {
        if (!ball->IsActive)
            continue;

        ball->Velocity *= frictionFactor;
    }
}

void Physics::IntegratePositions(std::vector<Ball*>& balls, float deltaTime)
{
    for (Ball* ball : balls)
    {
        if (!ball->IsActive)
            continue;

        ball->Update(deltaTime);
    }
}

void Physics::ResolveBallCollisions(std::vector<Ball*>& balls)
{
    int numBalls = (int)balls.size();

    for (int i = 0; i < numBalls; i++)
    {
        for (int j = i + 1; j < numBalls; j++)
        {
            Ball* a = balls[i];
            Ball* b = balls[j];

            if (!a->IsActive || !b->IsActive)
                continue;

            if (CheckBallCollision(a, b))
            {
                ResolveBallCollision(a, b);
            }
        }
    }
}

bool Physics::CheckBallCollision(const Ball* a, const Ball* b) const
{
    Vec3 diff = b->Position - a->Position;
    float distSq = diff.LengthSquared();
    float minDist = a->Radius + b->Radius;
    return distSq < minDist * minDist;
}

void Physics::ResolveBallCollision(Ball* a, Ball* b)
{
    // Vector from a to b
    Vec3 delta = b->Position - a->Position;
    float dist = delta.Length();

    if (dist < 0.0001f)
    {
        // Balls are at same position, push apart
        delta = Vec3(1.0f, 0.0f, 0.0f);
        dist = 0.0001f;
    }

    // Normal from a to b
    Vec3 normal = delta / dist;

    // Penetration depth
    float overlap = (a->Radius + b->Radius) - dist;

    // Separate the balls (push each half the overlap distance)
    Vec3 separation = normal * (overlap / 2.0f);
    a->Position -= separation;
    b->Position += separation;

    // Keep balls on the surface
    a->Position.y = a->Radius;
    b->Position.y = b->Radius;

    // Calculate relative velocity
    Vec3 relVel = a->Velocity - b->Velocity;

    // Relative velocity along collision normal
    float velAlongNormal = Dot(relVel, normal);

    // Only resolve if balls are approaching
    if (velAlongNormal > 0)
        return;

    // Calculate impulse scalar (assuming equal mass)
    // j = -(1 + e) * velAlongNormal / (1/m1 + 1/m2)
    // With equal masses: j = -(1 + e) * velAlongNormal / 2
    float e = PhysicsConstants::BALL_RESTITUTION;
    float j = -(1.0f + e) * velAlongNormal / 2.0f;

    // Apply impulse
    Vec3 impulse = normal * j;
    a->Velocity += impulse;
    b->Velocity -= impulse;
}

void Physics::ResolveCushionCollisions(std::vector<Ball*>& balls, const Table& table)
{
    float minX = table.GetMinX();
    float maxX = table.GetMaxX();
    float minZ = table.GetMinZ();
    float maxZ = table.GetMaxZ();

    float e = PhysicsConstants::CUSHION_RESTITUTION;

    for (Ball* ball : balls)
    {
        if (!ball->IsActive)
            continue;

        float r = ball->Radius;

        // Left cushion
        if (ball->Position.x - r < minX)
        {
            ball->Position.x = minX + r;
            if (ball->Velocity.x < 0)
                ball->Velocity.x = -ball->Velocity.x * e;
        }

        // Right cushion
        if (ball->Position.x + r > maxX)
        {
            ball->Position.x = maxX - r;
            if (ball->Velocity.x > 0)
                ball->Velocity.x = -ball->Velocity.x * e;
        }

        // Back cushion (near -Z)
        if (ball->Position.z - r < minZ)
        {
            ball->Position.z = minZ + r;
            if (ball->Velocity.z < 0)
                ball->Velocity.z = -ball->Velocity.z * e;
        }

        // Front cushion (near +Z)
        if (ball->Position.z + r > maxZ)
        {
            ball->Position.z = maxZ - r;
            if (ball->Velocity.z > 0)
                ball->Velocity.z = -ball->Velocity.z * e;
        }
    }
}

void Physics::ClampVelocities(std::vector<Ball*>& balls)
{
    for (Ball* ball : balls)
    {
        if (!ball->IsActive)
            continue;

        float speed = ball->Velocity.Length();
        if (speed > PhysicsConstants::MAX_VELOCITY)
        {
            ball->Velocity = ball->Velocity.Normalized() * PhysicsConstants::MAX_VELOCITY;
        }
    }
}

void Physics::StopSlowBalls(std::vector<Ball*>& balls)
{
    for (Ball* ball : balls)
    {
        if (!ball->IsActive)
            continue;

        if (ball->Velocity.Length() < PhysicsConstants::MIN_VELOCITY)
        {
            ball->Stop();
        }
    }
}
