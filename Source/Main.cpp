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
#include <cmath>

// ============================================================================
// CONSTANTS
// ============================================================================

const float TARGET_FPS = 75.0f;
const float TARGET_FRAME_TIME = 1.0f / TARGET_FPS;  // ~13.33ms

const float BALL_RADIUS = 0.057f;  // Standard pool ball radius (scaled)
const float MIN_SHOT_POWER = 1.0f;
const float MAX_SHOT_POWER = 8.0f;

// ============================================================================
// GLOBAL STATE
// ============================================================================

// Rendering toggles
bool g_DepthTestEnabled = true;
bool g_FaceCullingEnabled = true;

// Input state
bool g_KeyDPressed = false;
bool g_KeyCPressed = false;

// Mouse drag shooting state
bool g_IsDragging = false;
double g_MouseX = 0.0;
double g_MouseY = 0.0;
Vec3 g_MouseWorldPos;         // Current mouse position on table (Y=0)
bool g_MouseOnTable = false;  // Whether mouse projects onto table area

// Window dimensions (updated on resize)
int g_WindowWidth = 1920;
int g_WindowHeight = 1080;

// Camera pointer for mouse unprojection (set in main)
Camera* g_CameraPtr = nullptr;

// Forward declaration
Vec3 ScreenToWorld(double mouseX, double mouseY, int screenW, int screenH, const Camera& camera);

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
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            g_IsDragging = true;
        }
        else if (action == GLFW_RELEASE)
        {
            g_IsDragging = false;
        }
    }
}

void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    g_MouseX = xpos;
    g_MouseY = ypos;

    // Update world position if camera is available
    if (g_CameraPtr)
    {
        g_MouseWorldPos = ScreenToWorld(xpos, ypos, g_WindowWidth, g_WindowHeight, *g_CameraPtr);
        // Check if mouse is roughly over the table area
        g_MouseOnTable = (fabsf(g_MouseWorldPos.x) < 3.0f && fabsf(g_MouseWorldPos.z) < 4.0f);
    }
}

Vec3 ScreenToWorld(double mouseX, double mouseY, int screenW, int screenH, const Camera& camera)
{
    // Convert screen coords to NDC
    float ndcX = (2.0f * (float)mouseX / screenW) - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)mouseY / screenH);

    // Get inverse view-projection matrix
    Mat4 vp = camera.GetViewProjectionMatrix();
    Mat4 invVP = vp.Inverse();

    // Transform near and far points from clip space to world space
    auto transformPoint = [&invVP](float cx, float cy, float cz) -> Vec3 {
        float out[4];
        for (int i = 0; i < 4; i++)
            out[i] = invVP.m[i] * cx + invVP.m[4 + i] * cy + invVP.m[8 + i] * cz + invVP.m[12 + i] * 1.0f;
        if (fabsf(out[3]) > 0.0001f)
        {
            out[0] /= out[3];
            out[1] /= out[3];
            out[2] /= out[3];
        }
        return Vec3(out[0], out[1], out[2]);
    };

    Vec3 nearWorld = transformPoint(ndcX, ndcY, -1.0f);
    Vec3 farWorld = transformPoint(ndcX, ndcY, 1.0f);

    // Ray-plane intersection with Y=0
    Vec3 rayDir = farWorld - nearWorld;
    if (fabsf(rayDir.y) < 0.0001f)
        return Vec3(0.0f, 0.0f, 0.0f);

    float t = -nearWorld.y / rayDir.y;
    return Vec3(nearWorld.x + t * rayDir.x, 0.0f, nearWorld.z + t * rayDir.z);
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
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPositionCallback);

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
    g_CameraPtr = &camera;

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

    // Track whether mouse was dragging last frame (to detect release)
    bool wasDragging = false;

    // ==================== Main Loop ====================
    std::cout << "\n=== 3D Billiards ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    std::cout << "  D: Toggle depth testing" << std::endl;
    std::cout << "  C: Toggle face culling" << std::endl;
    std::cout << "  Mouse drag: Aim and shoot (drag from cue ball, further = harder)" << std::endl;
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

        // Handle mouse drag shooting
        // On release: shoot the cue ball in the direction from cue ball to mouse
        if (wasDragging && !g_IsDragging && physics.AllBallsStopped(balls))
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
                Vec3 diff = g_MouseWorldPos - Vec3(cueBall->Position.x, 0.0f, cueBall->Position.z);
                float dragDist = diff.Length();

                if (dragDist > 0.05f) // Minimum drag distance to shoot
                {
                    Vec3 shotDir = Vec3(diff.x, 0.0f, diff.z).Normalized();
                    float maxDragDist = 3.0f;
                    float power = MIN_SHOT_POWER + (MAX_SHOT_POWER - MIN_SHOT_POWER) * Clamp(dragDist / maxDragDist, 0.0f, 1.0f);
                    physics.ApplyImpulse(cueBall, shotDir, power);
                    std::cout << "Shot! Power: " << power << std::endl;
                }
            }
        }
        wasDragging = g_IsDragging;

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

        // Render aim line when dragging and balls are stopped
        if (g_IsDragging && physics.AllBallsStopped(balls))
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
                Vec3 cuePosXZ(cueBall->Position.x, 0.0f, cueBall->Position.z);
                Vec3 diff = g_MouseWorldPos - cuePosXZ;
                float dragDist = diff.Length();

                if (dragDist > 0.05f)
                {
                    Vec3 aimDir = diff.Normalized();
                    float aimAngle = atan2f(aimDir.x, aimDir.z);

                    // Color based on power: green (weak) -> yellow -> red (strong)
                    float powerFrac = Clamp(dragDist / 3.0f, 0.0f, 1.0f);
                    Vec3 aimColor;
                    if (powerFrac < 0.5f)
                    {
                        float t = powerFrac * 2.0f;
                        aimColor = Vec3(t, 1.0f, 0.0f); // green to yellow
                    }
                    else
                    {
                        float t = (powerFrac - 0.5f) * 2.0f;
                        aimColor = Vec3(1.0f, 1.0f - t, 0.0f); // yellow to red
                    }

                    // Draw aim indicator
                    billiardShader.SetFloat("uAmbient", 1.0f);
                    billiardShader.SetFloat("uSpecular", 0.0f);

                    float lineLen = 0.3f + powerFrac * 0.4f;

                    Mat4 aimModel = Mat4::Translate(cueBall->Position + aimDir * (cueBall->Radius + lineLen / 2.0f + 0.02f)) *
                                    Mat4::RotateY(-aimAngle) *
                                    Mat4::Scale(0.015f, 0.015f, lineLen);
                    Mat4 aimMVP = viewProjection * aimModel;

                    billiardShader.SetMat4("uMVP", aimMVP.Ptr());
                    billiardShader.SetMat4("uModel", aimModel.Ptr());
                    billiardShader.SetVec3("uObjectColor", aimColor.Ptr());

                    glBindVertexArray(g_AimVAO);
                    glDrawElements(GL_TRIANGLES, g_AimIndexCount, GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);

                    // Restore lighting
                    billiardShader.SetFloat("uAmbient", 0.3f);
                    billiardShader.SetFloat("uSpecular", 0.5f);
                }
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
