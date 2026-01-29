#include "../Header/Ball.h"
#include <cmath>

Ball::Ball(int number, const Vec3& position, float radius, const Vec3& color)
    : Number(number)
    , Position(position)
    , Velocity(0.0f, 0.0f, 0.0f)
    , Radius(radius)
    , Color(color)
    , IsActive(true)
    , VAO(0)
    , VBO(0)
    , EBO(0)
    , IndexCount(0)
{
}

Ball::~Ball()
{
    if (VAO != 0)
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
}

void Ball::InitMesh()
{
    // Generate sphere mesh
    MeshData mesh = GenerateSphereMesh(Radius, SPHERE_SECTORS, SPHERE_STACKS);
    IndexCount = (unsigned int)mesh.indices.size();

    // Create VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

    // Create EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    // Unbind
    glBindVertexArray(0);
}

void Ball::Render(Shader& shader, const Mat4& viewProjection)
{
    if (!IsActive || VAO == 0)
        return;

    // Calculate MVP matrix
    Mat4 model = GetModelMatrix();
    Mat4 mvp = viewProjection * model;

    // Set uniforms
    shader.SetMat4("uMVP", mvp.Ptr());
    shader.SetMat4("uModel", model.Ptr());
    shader.SetVec3("uObjectColor", Color.Ptr());

    // Draw
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
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
    return Mat4::Translate(Position);
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

    // Standard billiard ball colors (simplified - no stripes)
    // Colors are approximate RGB values

    // Ball colors: [number] = color
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

    // Standard triangle rack formation for 8-ball
    // The 8-ball should be in the center of the rack
    // Rack is positioned with apex toward the foot spot

    float diameter = ballRadius * 2.0f;
    float rowSpacing = diameter * 0.866f; // sin(60°) for tight packing

    // Rack starting position (apex of the triangle)
    float rackX = 0.0f;
    float rackZ = -1.5f; // Toward one end of the table

    // Ball arrangement in rows (front to back)
    // Row 0: 1 ball (apex)
    // Row 1: 2 balls
    // Row 2: 3 balls (8-ball in center)
    // Row 3: 4 balls
    // Row 4: 5 balls

    int ballOrder[] = {
        1,              // Row 0
        2, 3,           // Row 1
        4, 8, 5,        // Row 2 (8-ball center)
        6, 7, 9, 10,    // Row 3
        11, 12, 13, 14, 15  // Row 4
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

    // Add cue ball at the other end of the table
    Vec3 cuePos(0.0f, ballRadius, 2.5f);
    balls.insert(balls.begin(), new Ball(0, cuePos, ballRadius, colors[0]));

    return balls;
}
