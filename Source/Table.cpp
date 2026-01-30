#include "../Header/Table.h"

// Default dimensions based on standard 9-foot pool table (scaled)
Table::Table()
    : Width(2.5f)          // ~2.5 units wide (X)
    , Length(5.0f)         // ~5 units long (Z) - 2:1 ratio
    , CushionHeight(0.08f) // Height of cushion
    , CushionWidth(0.15f)  // Thickness of cushion
    , PocketRadius(0.12f)
    , SurfaceColor(0.0f, 0.4f, 0.0f)    // Green felt
    , CushionColor(0.4f, 0.25f, 0.1f)   // Brown rubber/wood
    , FrameColor(0.3f, 0.15f, 0.05f)    // Dark wood frame
    , SurfaceVAO(0), SurfaceVBO(0), SurfaceEBO(0), SurfaceIndexCount(0)
    , CushionVAO(0), CushionVBO(0), CushionEBO(0), CushionIndexCount(0)
    , FrameVAO(0), FrameVBO(0), FrameEBO(0), FrameIndexCount(0)
    , PocketVAO(0), PocketVBO(0), PocketEBO(0), PocketIndexCount(0)
{
    float hw = Width / 2.0f;
    float hl = Length / 2.0f;
    PocketPositions[0] = Vec3(-hw, 0.0f, -hl); // back-left corner
    PocketPositions[1] = Vec3( hw, 0.0f, -hl); // back-right corner
    PocketPositions[2] = Vec3(-hw, 0.0f,  hl); // front-left corner
    PocketPositions[3] = Vec3( hw, 0.0f,  hl); // front-right corner
    PocketPositions[4] = Vec3(-hw, 0.0f,  0.0f); // left side
    PocketPositions[5] = Vec3( hw, 0.0f,  0.0f); // right side
}

Table::Table(float width, float length, float cushionHeight, float cushionWidth)
    : Width(width)
    , Length(length)
    , CushionHeight(cushionHeight)
    , CushionWidth(cushionWidth)
    , PocketRadius(0.12f)
    , SurfaceColor(0.0f, 0.4f, 0.0f)
    , CushionColor(0.4f, 0.25f, 0.1f)
    , FrameColor(0.3f, 0.15f, 0.05f)
    , SurfaceVAO(0), SurfaceVBO(0), SurfaceEBO(0), SurfaceIndexCount(0)
    , CushionVAO(0), CushionVBO(0), CushionEBO(0), CushionIndexCount(0)
    , FrameVAO(0), FrameVBO(0), FrameEBO(0), FrameIndexCount(0)
    , PocketVAO(0), PocketVBO(0), PocketEBO(0), PocketIndexCount(0)
{
    float hw = Width / 2.0f;
    float hl = Length / 2.0f;
    PocketPositions[0] = Vec3(-hw, 0.0f, -hl);
    PocketPositions[1] = Vec3( hw, 0.0f, -hl);
    PocketPositions[2] = Vec3(-hw, 0.0f,  hl);
    PocketPositions[3] = Vec3( hw, 0.0f,  hl);
    PocketPositions[4] = Vec3(-hw, 0.0f,  0.0f);
    PocketPositions[5] = Vec3( hw, 0.0f,  0.0f);
}

Table::~Table()
{
    if (SurfaceVAO != 0)
    {
        glDeleteVertexArrays(1, &SurfaceVAO);
        glDeleteBuffers(1, &SurfaceVBO);
        glDeleteBuffers(1, &SurfaceEBO);
    }
    if (CushionVAO != 0)
    {
        glDeleteVertexArrays(1, &CushionVAO);
        glDeleteBuffers(1, &CushionVBO);
        glDeleteBuffers(1, &CushionEBO);
    }
    if (FrameVAO != 0)
    {
        glDeleteVertexArrays(1, &FrameVAO);
        glDeleteBuffers(1, &FrameVBO);
        glDeleteBuffers(1, &FrameEBO);
    }
    if (PocketVAO != 0)
    {
        glDeleteVertexArrays(1, &PocketVAO);
        glDeleteBuffers(1, &PocketVBO);
        glDeleteBuffers(1, &PocketEBO);
    }
}

void Table::InitMesh()
{
    GenerateSurfaceMesh();
    GenerateCushionMesh();
    GenerateFrameMesh();
    GeneratePocketMesh();
}

void Table::Render(Shader& shader, const Mat4& viewProjection)
{
    Mat4 model = Mat4::Identity();
    Mat4 mvp = viewProjection * model;

    shader.SetMat4("uMVP", mvp.Ptr());
    shader.SetMat4("uModel", model.Ptr());

    // Render surface
    shader.SetVec3("uObjectColor", SurfaceColor.Ptr());
    glBindVertexArray(SurfaceVAO);
    glDrawElements(GL_TRIANGLES, SurfaceIndexCount, GL_UNSIGNED_INT, 0);

    // Render cushions
    shader.SetVec3("uObjectColor", CushionColor.Ptr());
    glBindVertexArray(CushionVAO);
    glDrawElements(GL_TRIANGLES, CushionIndexCount, GL_UNSIGNED_INT, 0);

    // Render frame
    shader.SetVec3("uObjectColor", FrameColor.Ptr());
    glBindVertexArray(FrameVAO);
    glDrawElements(GL_TRIANGLES, FrameIndexCount, GL_UNSIGNED_INT, 0);

    // Render pockets (dark circles)
    Vec3 pocketColor(0.02f, 0.02f, 0.02f);
    shader.SetVec3("uObjectColor", pocketColor.Ptr());
    glBindVertexArray(PocketVAO);
    for (int i = 0; i < NUM_POCKETS; i++)
    {
        Mat4 pocketModel = Mat4::Translate(PocketPositions[i].x, 0.002f, PocketPositions[i].z);
        Mat4 pocketMVP = viewProjection * pocketModel;
        shader.SetMat4("uMVP", pocketMVP.Ptr());
        shader.SetMat4("uModel", pocketModel.Ptr());
        glDrawElements(GL_TRIANGLES, PocketIndexCount, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

Vec3 Table::GetPlayAreaHalfExtents() const
{
    // Inner playing area (inside cushions)
    return Vec3(Width / 2.0f, 0.0f, Length / 2.0f);
}

float Table::GetMinX() const
{
    return -Width / 2.0f;
}

float Table::GetMaxX() const
{
    return Width / 2.0f;
}

float Table::GetMinZ() const
{
    return -Length / 2.0f;
}

float Table::GetMaxZ() const
{
    return Length / 2.0f;
}

const Vec3* Table::GetPocketPositions() const
{
    return PocketPositions;
}

float Table::GetPocketRadius() const
{
    return PocketRadius;
}

void Table::GenerateSurfaceMesh()
{
    MeshData mesh;

    float hw = Width / 2.0f;
    float hl = Length / 2.0f;

    // Playing surface is a flat quad at Y = 0
    Vertex v0, v1, v2, v3;

    // Bottom-left (looking down from +Y)
    v0.position[0] = -hw; v0.position[1] = 0.0f; v0.position[2] = -hl;
    v0.normal[0] = 0.0f;  v0.normal[1] = 1.0f;   v0.normal[2] = 0.0f;
    v0.texCoord[0] = 0.0f; v0.texCoord[1] = 0.0f;

    // Bottom-right
    v1.position[0] = hw;  v1.position[1] = 0.0f; v1.position[2] = -hl;
    v1.normal[0] = 0.0f;  v1.normal[1] = 1.0f;   v1.normal[2] = 0.0f;
    v1.texCoord[0] = 1.0f; v1.texCoord[1] = 0.0f;

    // Top-right
    v2.position[0] = hw;  v2.position[1] = 0.0f; v2.position[2] = hl;
    v2.normal[0] = 0.0f;  v2.normal[1] = 1.0f;   v2.normal[2] = 0.0f;
    v2.texCoord[0] = 1.0f; v2.texCoord[1] = 1.0f;

    // Top-left
    v3.position[0] = -hw; v3.position[1] = 0.0f; v3.position[2] = hl;
    v3.normal[0] = 0.0f;  v3.normal[1] = 1.0f;   v3.normal[2] = 0.0f;
    v3.texCoord[0] = 0.0f; v3.texCoord[1] = 1.0f;

    mesh.vertices.push_back(v0);
    mesh.vertices.push_back(v1);
    mesh.vertices.push_back(v2);
    mesh.vertices.push_back(v3);

    mesh.indices.push_back(0);
    mesh.indices.push_back(1);
    mesh.indices.push_back(2);
    mesh.indices.push_back(0);
    mesh.indices.push_back(2);
    mesh.indices.push_back(3);

    UploadMesh(mesh, SurfaceVAO, SurfaceVBO, SurfaceEBO, SurfaceIndexCount);
}

void Table::GenerateCushionMesh()
{
    MeshData mesh;

    float hw = Width / 2.0f;
    float hl = Length / 2.0f;
    float ch = CushionHeight;
    float cw = CushionWidth;

    // Helper lambda to add a box to the mesh
    auto addBox = [&mesh](float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        unsigned int baseIndex = (unsigned int)mesh.vertices.size();

        // 8 corners of the box
        Vec3 corners[8] = {
            Vec3(minX, minY, minZ), // 0: left-bottom-back
            Vec3(maxX, minY, minZ), // 1: right-bottom-back
            Vec3(maxX, maxY, minZ), // 2: right-top-back
            Vec3(minX, maxY, minZ), // 3: left-top-back
            Vec3(minX, minY, maxZ), // 4: left-bottom-front
            Vec3(maxX, minY, maxZ), // 5: right-bottom-front
            Vec3(maxX, maxY, maxZ), // 6: right-top-front
            Vec3(minX, maxY, maxZ)  // 7: left-top-front
        };

        // 6 faces with normals
        struct Face { int v[4]; Vec3 normal; };
        Face faces[6] = {
            { {4, 5, 6, 7}, Vec3(0, 0, 1) },   // Front (+Z)
            { {1, 0, 3, 2}, Vec3(0, 0, -1) },  // Back (-Z)
            { {5, 1, 2, 6}, Vec3(1, 0, 0) },   // Right (+X)
            { {0, 4, 7, 3}, Vec3(-1, 0, 0) },  // Left (-X)
            { {7, 6, 2, 3}, Vec3(0, 1, 0) },   // Top (+Y)
            { {0, 1, 5, 4}, Vec3(0, -1, 0) }   // Bottom (-Y)
        };

        for (int f = 0; f < 6; f++)
        {
            unsigned int faceBase = (unsigned int)mesh.vertices.size();

            for (int i = 0; i < 4; i++)
            {
                Vertex v;
                v.position[0] = corners[faces[f].v[i]].x;
                v.position[1] = corners[faces[f].v[i]].y;
                v.position[2] = corners[faces[f].v[i]].z;
                v.normal[0] = faces[f].normal.x;
                v.normal[1] = faces[f].normal.y;
                v.normal[2] = faces[f].normal.z;
                v.texCoord[0] = (i == 1 || i == 2) ? 1.0f : 0.0f;
                v.texCoord[1] = (i == 2 || i == 3) ? 1.0f : 0.0f;
                mesh.vertices.push_back(v);
            }

            // Two triangles per face
            mesh.indices.push_back(faceBase + 0);
            mesh.indices.push_back(faceBase + 1);
            mesh.indices.push_back(faceBase + 2);
            mesh.indices.push_back(faceBase + 0);
            mesh.indices.push_back(faceBase + 2);
            mesh.indices.push_back(faceBase + 3);
        }
    };

    // Cushions with gaps at pocket locations
    // pg = pocket gap size (how much cushion is removed at each pocket)
    float pg = PocketRadius * 2.0f;

    // Left cushion (-X side): split into 2 segments with gaps at corners and side pocket
    addBox(-hw - cw, 0, -hl + pg, -hw, ch, -pg);   // left segment 1 (back corner to side pocket)
    addBox(-hw - cw, 0,  pg,      -hw, ch, hl - pg); // left segment 2 (side pocket to front corner)

    // Right cushion (+X side): same pattern
    addBox(hw, 0, -hl + pg, hw + cw, ch, -pg);      // right segment 1
    addBox(hw, 0,  pg,      hw + cw, ch, hl - pg);   // right segment 2

    // Back cushion (-Z side): one segment with corner gaps
    addBox(-hw + pg, 0, -hl - cw, hw - pg, ch, -hl);

    // Front cushion (+Z side): one segment with corner gaps
    addBox(-hw + pg, 0, hl, hw - pg, ch, hl + cw);

    UploadMesh(mesh, CushionVAO, CushionVBO, CushionEBO, CushionIndexCount);
}

void Table::GenerateFrameMesh()
{
    MeshData mesh;

    float hw = Width / 2.0f;
    float hl = Length / 2.0f;
    float ch = CushionHeight;
    float cw = CushionWidth;
    float frameWidth = 0.1f;
    float frameDepth = 0.1f;

    // Helper lambda (same as cushion)
    auto addBox = [&mesh](float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        Vec3 corners[8] = {
            Vec3(minX, minY, minZ),
            Vec3(maxX, minY, minZ),
            Vec3(maxX, maxY, minZ),
            Vec3(minX, maxY, minZ),
            Vec3(minX, minY, maxZ),
            Vec3(maxX, minY, maxZ),
            Vec3(maxX, maxY, maxZ),
            Vec3(minX, maxY, maxZ)
        };

        struct Face { int v[4]; Vec3 normal; };
        Face faces[6] = {
            { {4, 5, 6, 7}, Vec3(0, 0, 1) },
            { {1, 0, 3, 2}, Vec3(0, 0, -1) },
            { {5, 1, 2, 6}, Vec3(1, 0, 0) },
            { {0, 4, 7, 3}, Vec3(-1, 0, 0) },
            { {7, 6, 2, 3}, Vec3(0, 1, 0) },
            { {0, 1, 5, 4}, Vec3(0, -1, 0) }
        };

        for (int f = 0; f < 6; f++)
        {
            unsigned int faceBase = (unsigned int)mesh.vertices.size();

            for (int i = 0; i < 4; i++)
            {
                Vertex v;
                v.position[0] = corners[faces[f].v[i]].x;
                v.position[1] = corners[faces[f].v[i]].y;
                v.position[2] = corners[faces[f].v[i]].z;
                v.normal[0] = faces[f].normal.x;
                v.normal[1] = faces[f].normal.y;
                v.normal[2] = faces[f].normal.z;
                v.texCoord[0] = (i == 1 || i == 2) ? 1.0f : 0.0f;
                v.texCoord[1] = (i == 2 || i == 3) ? 1.0f : 0.0f;
                mesh.vertices.push_back(v);
            }

            mesh.indices.push_back(faceBase + 0);
            mesh.indices.push_back(faceBase + 1);
            mesh.indices.push_back(faceBase + 2);
            mesh.indices.push_back(faceBase + 0);
            mesh.indices.push_back(faceBase + 2);
            mesh.indices.push_back(faceBase + 3);
        }
    };

    // Outer frame (decorative) - below the cushions
    float outerX = hw + cw + frameWidth;
    float outerZ = hl + cw + frameWidth;

    // Frame goes from below surface to cushion top
    float frameBottom = -frameDepth;
    float frameTop = ch;

    // Left frame
    addBox(-outerX, frameBottom, -outerZ, -hw - cw, frameTop, outerZ);

    // Right frame
    addBox(hw + cw, frameBottom, -outerZ, outerX, frameTop, outerZ);

    // Back frame (between left and right)
    addBox(-hw - cw, frameBottom, -outerZ, hw + cw, frameTop, -hl - cw);

    // Front frame (between left and right)
    addBox(-hw - cw, frameBottom, hl + cw, hw + cw, frameTop, outerZ);

    UploadMesh(mesh, FrameVAO, FrameVBO, FrameEBO, FrameIndexCount);
}

void Table::GeneratePocketMesh()
{
    MeshData mesh = GenerateDiscMesh(PocketRadius, 32);
    UploadMesh(mesh, PocketVAO, PocketVBO, PocketEBO, PocketIndexCount);
}

void Table::UploadMesh(const MeshData& mesh, GLuint& vao, GLuint& vbo, GLuint& ebo, unsigned int& indexCount)
{
    indexCount = (unsigned int)mesh.indices.size();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // TexCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}
