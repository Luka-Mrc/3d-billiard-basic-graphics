#include "../Header/Ball.h"
#include <cmath>
#include <iostream>

// Static member initialization
Model* Ball::s_SphereModel = nullptr;

Ball::Ball(int number, const Vec3& position, float radius, const Vec3& color)
    : Number(number)
    , Position(position)
    , Velocity(0.0f, 0.0f, 0.0f)
    , Radius(radius)
    , Color(color)
    , IsActive(true)
{
}

Ball::~Ball()
{
}

void Ball::LoadModel(const std::string& path)
{
    if (s_SphereModel == nullptr)
    {
        std::cout << "Loading sphere model: " << path << std::endl;
        s_SphereModel = new Model(path);
        std::cout << "Sphere model loaded successfully" << std::endl;
    }
}

void Ball::CleanupModel()
{
    if (s_SphereModel != nullptr)
    {
        delete s_SphereModel;
        s_SphereModel = nullptr;
    }
}

void Ball::Render(Shader& shader, const Mat4& viewProjection)
{
    if (!IsActive || s_SphereModel == nullptr)
        return;

    // Calculate MVP matrix
    Mat4 model = GetModelMatrix();
    Mat4 mvp = viewProjection * model;

    // Set uniforms
    shader.SetMat4("uMVP", mvp.Ptr());
    shader.SetMat4("uModel", model.Ptr());
    shader.SetVec3("uObjectColor", Color.Ptr());

    // Draw using the shared 3D model
    s_SphereModel->Draw(shader);
}

void Ball::Update(float deltaTime)
{
    if (!IsActive)
        return;

    // Simple Euler integration
    Position += Velocity * deltaTime;

    // Keep Y at radius (ball rests on surface)
    Position.y = Radius;
}

Mat4 Ball::GetModelMatrix() const
{
    // Translate to position and scale from unit sphere to ball radius
    return Mat4::Translate(Position) * Mat4::Scale(Radius);
}

bool Ball::IsMoving() const
{
    return Velocity.LengthSquared() > VELOCITY_THRESHOLD * VELOCITY_THRESHOLD;
}

void Ball::Stop()
{
    Velocity = Vec3(0.0f, 0.0f, 0.0f);
}

// ============================================================================
// BALL SET CREATION
// ============================================================================

std::vector<Ball*> CreateStandardBallSet(float ballRadius)
{
    std::vector<Ball*> balls;

    Vec3 colors[] = {
        Vec3(1.0f, 1.0f, 1.0f),    // 0: Cue ball (white)
        Vec3(1.0f, 0.85f, 0.0f),   // 1: Yellow
        Vec3(0.0f, 0.0f, 0.8f),    // 2: Blue
        Vec3(1.0f, 0.0f, 0.0f),    // 3: Red
        Vec3(0.5f, 0.0f, 0.5f),    // 4: Purple
        Vec3(1.0f, 0.5f, 0.0f),    // 5: Orange
        Vec3(0.0f, 0.5f, 0.0f),    // 6: Green
        Vec3(0.5f, 0.0f, 0.0f),    // 7: Maroon
        Vec3(0.1f, 0.1f, 0.1f),    // 8: Black (8-ball)
        Vec3(1.0f, 0.85f, 0.4f),   // 9: Yellow stripe (lighter)
        Vec3(0.3f, 0.3f, 0.9f),    // 10: Blue stripe (lighter)
        Vec3(1.0f, 0.4f, 0.4f),    // 11: Red stripe (lighter)
        Vec3(0.7f, 0.3f, 0.7f),    // 12: Purple stripe (lighter)
        Vec3(1.0f, 0.7f, 0.3f),    // 13: Orange stripe (lighter)
        Vec3(0.3f, 0.7f, 0.3f),    // 14: Green stripe (lighter)
        Vec3(0.7f, 0.3f, 0.3f)     // 15: Maroon stripe (lighter)
    };

    float diameter = ballRadius * 2.0f;
    float rowSpacing = diameter * 0.866f;

    float rackX = 0.0f;
    float rackZ = -1.5f;

    int ballOrder[] = {
        1,
        2, 3,
        4, 8, 5,
        6, 7, 9, 10,
        11, 12, 13, 14, 15
    };

    int ballIndex = 0;
    for (int row = 0; row < 5; row++)
    {
        int ballsInRow = row + 1;
        float rowZ = rackZ - row * rowSpacing;
        float startX = rackX - (ballsInRow - 1) * ballRadius;

        for (int i = 0; i < ballsInRow; i++)
        {
            int num = ballOrder[ballIndex];
            float x = startX + i * diameter;
            Vec3 pos(x, ballRadius, rowZ);
            balls.push_back(new Ball(num, pos, ballRadius, colors[num]));
            ballIndex++;
        }
    }

    // Add cue ball
    Vec3 cuePos(0.0f, ballRadius, 2.5f);
    balls.insert(balls.begin(), new Ball(0, cuePos, ballRadius, colors[0]));

    return balls;
}
