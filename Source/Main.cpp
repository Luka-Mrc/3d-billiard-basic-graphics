/**
 * 3D Billiards Application
 * ========================
 * A complete 3D billiards game using modern OpenGL 3.3+
 *
 * Controls:
 * - ESC: Exit application
 * - D: Toggle depth testing
 * - C: Toggle face culling
 * - SPACE: Hit cue ball (when balls stopped)
 * - Arrow keys: Aim cue direction
 * - W/S: Adjust shot power
 *
 * Requirements met:
 * - Modern OpenGL (VAO, VBO, shaders)
 * - Fullscreen rendering
 * - 75 FPS frame limiter
 * - Runtime toggles for depth/culling
 * - Semi-transparent overlay texture
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Header/Shader.h"
#include "../Header/Camera.h"
#include "../Header/Table.h"
#include "../Header/Ball.h"
#include "../Header/Physics.h"
#include "../Header/Util.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

// ============================================================================
// CONSTANTS
// ============================================================================

const float TARGET_FPS = 75.0f;
const float TARGET_FRAME_TIME = 1.0f / TARGET_FPS;  // ~13.33ms

const float BALL_RADIUS = 0.057f;  // Standard pool ball radius (scaled)
const float MIN_SHOT_POWER = 1.0f;
const float MAX_SHOT_POWER = 8.0f;
const float SHOT_POWER_STEP = 0.5f;

// ============================================================================
// GLOBAL STATE
// ============================================================================

// Rendering toggles
bool g_DepthTestEnabled = true;
bool g_FaceCullingEnabled = true;

// Input state
bool g_KeyDPressed = false;
bool g_KeyCPressed = false;
bool g_KeySpacePressed = false;

// Shot aiming
float g_AimAngle = 0.0f;           // Radians
float g_ShotPower = 3.0f;          // Current shot power

// Window dimensions (updated on resize)
int g_WindowWidth = 1920;
int g_WindowHeight = 1080;

// ============================================================================
// OVERLAY QUAD FOR TEXTURE
// ============================================================================

GLuint g_OverlayVAO = 0;
GLuint g_OverlayVBO = 0;
GLuint g_OverlayEBO = 0;

// ============================================================================
// AIM INDICATOR BOX
// ============================================================================

GLuint g_AimVAO = 0;
GLuint g_AimVBO = 0;
GLuint g_AimEBO = 0;
unsigned int g_AimIndexCount = 0;

void InitOverlayQuad()
{
    MeshData quad = GenerateQuadMesh();

    glGenVertexArrays(1, &g_OverlayVAO);
    glBindVertexArray(g_OverlayVAO);

    glGenBuffers(1, &g_OverlayVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_OverlayVBO);
    glBufferData(GL_ARRAY_BUFFER, quad.vertices.size() * sizeof(Vertex), quad.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &g_OverlayEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_OverlayEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad.indices.size() * sizeof(unsigned int), quad.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void CleanupOverlayQuad()
{
    if (g_OverlayVAO != 0)
    {
        glDeleteVertexArrays(1, &g_OverlayVAO);
        glDeleteBuffers(1, &g_OverlayVBO);
        glDeleteBuffers(1, &g_OverlayEBO);
    }
}

void InitAimIndicator()
{
    // Create a unit box (1x1x1) centered at origin - will be scaled when rendered
    MeshData box = GenerateBoxMesh(1.0f, 1.0f, 1.0f);
    g_AimIndexCount = (unsigned int)box.indices.size();

    glGenVertexArrays(1, &g_AimVAO);
    glBindVertexArray(g_AimVAO);

    glGenBuffers(1, &g_AimVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_AimVBO);
    glBufferData(GL_ARRAY_BUFFER, box.vertices.size() * sizeof(Vertex), box.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &g_AimEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_AimEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, box.indices.size() * sizeof(unsigned int), box.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void CleanupAimIndicator()
{
    if (g_AimVAO != 0)
    {
        glDeleteVertexArrays(1, &g_AimVAO);
        glDeleteBuffers(1, &g_AimVBO);
        glDeleteBuffers(1, &g_AimEBO);
    }
}

// ============================================================================
// CALLBACKS
// ============================================================================

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    g_WindowWidth = width;
    g_WindowHeight = height;
    glViewport(0, 0, width, height);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // ESC to exit
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // D to toggle depth testing
    if (key == GLFW_KEY_D && action == GLFW_PRESS && !g_KeyDPressed)
    {
        g_DepthTestEnabled = !g_DepthTestEnabled;
        std::cout << "Depth testing: " << (g_DepthTestEnabled ? "ON" : "OFF") << std::endl;
    }
    g_KeyDPressed = (key == GLFW_KEY_D && action != GLFW_RELEASE);

    // C to toggle face culling
    if (key == GLFW_KEY_C && action == GLFW_PRESS && !g_KeyCPressed)
    {
        g_FaceCullingEnabled = !g_FaceCullingEnabled;
        std::cout << "Face culling: " << (g_FaceCullingEnabled ? "ON" : "OFF") << std::endl;
    }
    g_KeyCPressed = (key == GLFW_KEY_C && action != GLFW_RELEASE);

    // Space to shoot
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        g_KeySpacePressed = true;
    }

    // Arrow keys for aiming
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_LEFT)
            g_AimAngle += 0.05f;
        if (key == GLFW_KEY_RIGHT)
            g_AimAngle -= 0.05f;
    }

    // W/S for power adjustment
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_W)
            g_ShotPower = Clamp(g_ShotPower + SHOT_POWER_STEP, MIN_SHOT_POWER, MAX_SHOT_POWER);
        if (key == GLFW_KEY_S)
            g_ShotPower = Clamp(g_ShotPower - SHOT_POWER_STEP, MIN_SHOT_POWER, MAX_SHOT_POWER);
    }
}

// ============================================================================
// RENDERING HELPERS
// ============================================================================

void ApplyRenderState()
{
    if (g_DepthTestEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (g_FaceCullingEnabled)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void RenderOverlay(Shader& overlayShader, GLuint textureID, float alpha)
{
    // Disable depth testing for 2D overlay
    glDisable(GL_DEPTH_TEST);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    overlayShader.Use();

    // Position in bottom-right corner
    // NDC: (-1,-1) is bottom-left, (1,1) is top-right
    float overlayWidth = 0.3f;   // 30% of screen width
    float overlayHeight = 0.15f; // 15% of screen height
    float posX = 1.0f - overlayWidth - 0.02f;   // Right side with margin
    float posY = -1.0f + 0.02f;                  // Bottom with margin

    overlayShader.SetVec2("uPosition", posX, posY);
    overlayShader.SetVec2("uSize", overlayWidth, overlayHeight);
    overlayShader.SetFloat("uAlpha", alpha);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    overlayShader.SetInt("uTexture", 0);

    // Draw quad
    glBindVertexArray(g_OverlayVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Restore depth testing if it was enabled
    if (g_DepthTestEnabled)
        glEnable(GL_DEPTH_TEST);
}

// ============================================================================
// MAIN
// ============================================================================

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure OpenGL context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Get primary monitor for fullscreen
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // Create fullscreen window
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "3D Billiards - Efren Reyes Edition", monitor, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetKeyCallback(window, KeyCallback);

    // Disable vsync (we'll do our own frame limiting)
    glfwSwapInterval(0);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Get initial window size
    glfwGetFramebufferSize(window, &g_WindowWidth, &g_WindowHeight);
    glViewport(0, 0, g_WindowWidth, g_WindowHeight);

    // Print OpenGL info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // ==================== Load Shaders ====================
    Shader billiardShader;
    if (!billiardShader.Load("Shaders/billiard.vert", "Shaders/billiard.frag"))
    {
        std::cerr << "Failed to load billiard shader" << std::endl;
        glfwTerminate();
        return -1;
    }

    Shader overlayShader;
    if (!overlayShader.Load("Shaders/overlay.vert", "Shaders/overlay.frag"))
    {
        std::cerr << "Failed to load overlay shader" << std::endl;
        glfwTerminate();
        return -1;
    }

    // ==================== Load Textures ====================
    GLuint overlayTexture = LoadTexture("Resources/efren_reyes.png", true);
    if (overlayTexture == 0)
    {
        std::cout << "Warning: Could not load overlay texture, continuing without it" << std::endl;
    }

    // ==================== Initialize Game Objects ====================

    // Camera - positioned above and behind the table
    Camera camera;
    camera.SetPosition(0.0f, 6.0f, 5.0f);
    camera.SetTarget(0.0f, 0.0f, 0.0f);
    camera.SetPerspective(45.0f, (float)g_WindowWidth / (float)g_WindowHeight, 0.1f, 100.0f);

    // Table
    Table table;
    table.InitMesh();

    // Balls
    std::vector<Ball*> balls = CreateStandardBallSet(BALL_RADIUS);
    for (Ball* ball : balls)
    {
        ball->InitMesh();
    }

    // Physics
    Physics physics;

    // Overlay quad and aim indicator
    InitOverlayQuad();
    InitAimIndicator();

    // ==================== Lighting Setup ====================
    Vec3 lightDir = Vec3(0.3f, 1.0f, 0.5f).Normalized();  // Light from above-front
    Vec3 lightColor(1.0f, 1.0f, 0.95f);  // Slightly warm white

    // ==================== Main Loop ====================
    std::cout << "\n=== 3D Billiards ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    std::cout << "  D: Toggle depth testing" << std::endl;
    std::cout << "  C: Toggle face culling" << std::endl;
    std::cout << "  SPACE: Shoot cue ball" << std::endl;
    std::cout << "  LEFT/RIGHT: Aim" << std::endl;
    std::cout << "  W/S: Adjust power" << std::endl;
    std::cout << "===================\n" << std::endl;

    auto lastTime = std::chrono::high_resolution_clock::now();
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        // ============ Frame Timing ============
        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // ============ Input ============
        glfwPollEvents();

        // Handle shooting
        if (g_KeySpacePressed && physics.AllBallsStopped(balls))
        {
            g_KeySpacePressed = false;

            // Find cue ball (ball 0)
            Ball* cueBall = nullptr;
            for (Ball* ball : balls)
            {
                if (ball->Number == 0 && ball->IsActive)
                {
                    cueBall = ball;
                    break;
                }
            }

            if (cueBall)
            {
                // Calculate shot direction from aim angle
                Vec3 shotDir(sinf(g_AimAngle), 0.0f, cosf(g_AimAngle));
                physics.ApplyImpulse(cueBall, shotDir, g_ShotPower);
                std::cout << "Shot! Power: " << g_ShotPower << std::endl;
            }
        }

        // ============ Update ============
        physics.Update(balls, table, deltaTime);

        // Update camera aspect ratio if window was resized
        camera.SetAspectRatio((float)g_WindowWidth / (float)g_WindowHeight);

        // ============ Render ============
        // Clear buffers
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);  // Dark blue-gray background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Apply render state (depth test, face culling)
        ApplyRenderState();

        // Get view-projection matrix
        Mat4 viewProjection = camera.GetViewProjectionMatrix();

        // Use billiard shader
        billiardShader.Use();

        // Set lighting uniforms
        billiardShader.SetVec3("uLightDir", lightDir.Ptr());
        billiardShader.SetVec3("uLightColor", lightColor.Ptr());
        billiardShader.SetVec3("uViewPos", camera.Position.Ptr());
        billiardShader.SetFloat("uAmbient", 0.3f);
        billiardShader.SetFloat("uSpecular", 0.5f);
        billiardShader.SetFloat("uShininess", 32.0f);

        // Render table
        table.Render(billiardShader, viewProjection);

        // Render balls
        for (Ball* ball : balls)
        {
            ball->Render(billiardShader, viewProjection);
        }

        // Render aim line when balls are stopped
        if (physics.AllBallsStopped(balls))
        {
            Ball* cueBall = nullptr;
            for (Ball* ball : balls)
            {
                if (ball->Number == 0 && ball->IsActive)
                {
                    cueBall = ball;
                    break;
                }
            }

            if (cueBall)
            {
                // Draw aim indicator
                billiardShader.SetFloat("uAmbient", 1.0f);  // Full brightness for aim line
                billiardShader.SetFloat("uSpecular", 0.0f);

                // Simple aim visualization using a stretched cube
                Vec3 aimStart = cueBall->Position;
                Vec3 aimDir(sinf(g_AimAngle), 0.0f, cosf(g_AimAngle));
                float lineLen = 0.3f + (g_ShotPower / MAX_SHOT_POWER) * 0.4f;

                Mat4 aimModel = Mat4::Translate(aimStart + aimDir * (cueBall->Radius + lineLen / 2.0f + 0.02f)) *
                                Mat4::RotateY(-g_AimAngle) *
                                Mat4::Scale(0.015f, 0.015f, lineLen);
                Mat4 aimMVP = viewProjection * aimModel;

                billiardShader.SetMat4("uMVP", aimMVP.Ptr());
                billiardShader.SetMat4("uModel", aimModel.Ptr());
                billiardShader.SetVec3("uObjectColor", 1.0f, 1.0f, 0.2f);  // Yellow aim line

                // Draw the aim indicator box
                glBindVertexArray(g_AimVAO);
                glDrawElements(GL_TRIANGLES, g_AimIndexCount, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                // Restore lighting for next frame
                billiardShader.SetFloat("uAmbient", 0.3f);
                billiardShader.SetFloat("uSpecular", 0.5f);
            }
        }

        // Render overlay texture (semi-transparent)
        if (overlayTexture != 0)
        {
            RenderOverlay(overlayShader, overlayTexture, 0.7f);
        }

        // ============ Swap & Frame Limit ============
        glfwSwapBuffers(window);

        // Frame limiter to exactly 75 FPS
        auto frameEnd = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float>(frameEnd - currentTime).count();

        if (frameTime < TARGET_FRAME_TIME)
        {
            float sleepTime = TARGET_FRAME_TIME - frameTime;
            std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
        }
    }

    // ==================== Cleanup ====================
    std::cout << "Cleaning up..." << std::endl;

    // Delete balls
    for (Ball* ball : balls)
    {
        delete ball;
    }
    balls.clear();

    // Delete overlay and aim indicator
    CleanupOverlayQuad();
    CleanupAimIndicator();

    // Delete texture
    if (overlayTexture != 0)
    {
        glDeleteTextures(1, &overlayTexture);
    }

    // GLFW cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Goodbye!" << std::endl;
    return 0;
}
