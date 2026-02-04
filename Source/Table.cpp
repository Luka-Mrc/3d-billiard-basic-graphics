#include "../Header/Table.h"

// Default dimensions based on standard 9-foot pool table (scaled)
Table::Table()
    : Width(2.5f)          // ~2.5 units wide (X)
    , Length(5.0f)         // ~5 units long (Z) - 2:1 ratio
    , CushionHeight(0.08f) // Height of cushion
    , CushionWidth(0.15f)  // Thickness of cushion
    , PocketRadius(0.12f)
    , SurfaceColor(0.05f, 0.5f, 0.1f)     // Rich green felt
    , CushionColor(0.04f, 0.42f, 0.08f)   // Green felt on cushions
    , FrameColor(0.35f, 0.2f, 0.08f)      // Warm dark wood frame
    , SurfaceVAO(0), SurfaceVBO(0), SurfaceEBO(0), SurfaceIndexCount(0)
    , CushionVAO(0), CushionVBO(0), CushionEBO(0), CushionIndexCount(0)
    , FrameVAO(0), FrameVBO(0), FrameEBO(0), FrameIndexCount(0)
    , PocketVAO(0), PocketVBO(0), PocketEBO(0), PocketIndexCount(0)
    , PocketRimVAO(0), PocketRimVBO(0), PocketRimEBO(0), PocketRimIndexCount(0)
{
    float hw = Width / 2.0f;
    float hl = Length / 2.0f;
    // Pockets shifted slightly into the cushion/frame area for a more embedded look
    float co = 0.03f; // corner offset (diagonal)
    float so = 0.04f; // side offset (perpendicular to rail)
    PocketPositions[0] = Vec3(-hw - co, 0.0f, -hl - co); // back-left corner
    PocketPositions[1] = Vec3( hw + co, 0.0f, -hl - co); // back-right corner
    PocketPositions[2] = Vec3(-hw - co, 0.0f,  hl + co); // front-left corner
    PocketPositions[3] = Vec3( hw + co, 0.0f,  hl + co); // front-right corner
    PocketPositions[4] = Vec3(-hw - so, 0.0f,  0.0f);    // left side
    PocketPositions[5] = Vec3( hw + so, 0.0f,  0.0f);    // right side
}

Table::Table(float width, float length, float cushionHeight, float cushionWidth)
    : Width(width)
    , Length(length)
    , CushionHeight(cushionHeight)
    , CushionWidth(cushionWidth)
    , PocketRadius(0.12f)
    , SurfaceColor(0.05f, 0.5f, 0.1f)
    , CushionColor(0.04f, 0.42f, 0.08f)
    , FrameColor(0.35f, 0.2f, 0.08f)
    , SurfaceVAO(0), SurfaceVBO(0), SurfaceEBO(0), SurfaceIndexCount(0)
    , CushionVAO(0), CushionVBO(0), CushionEBO(0), CushionIndexCount(0)
    , FrameVAO(0), FrameVBO(0), FrameEBO(0), FrameIndexCount(0)
    , PocketVAO(0), PocketVBO(0), PocketEBO(0), PocketIndexCount(0)
    , PocketRimVAO(0), PocketRimVBO(0), PocketRimEBO(0), PocketRimIndexCount(0)
{
    float hw = Width / 2.0f;
    float hl = Length / 2.0f;
    float co = 0.03f;
    float so = 0.04f;
    PocketPositions[0] = Vec3(-hw - co, 0.0f, -hl - co);
    PocketPositions[1] = Vec3( hw + co, 0.0f, -hl - co);
    PocketPositions[2] = Vec3(-hw - co, 0.0f,  hl + co);
    PocketPositions[3] = Vec3( hw + co, 0.0f,  hl + co);
    PocketPositions[4] = Vec3(-hw - so, 0.0f,  0.0f);
    PocketPositions[5] = Vec3( hw + so, 0.0f,  0.0f);
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
    if (PocketRimVAO != 0)
    {
        glDeleteVertexArrays(1, &PocketRimVAO);
        glDeleteBuffers(1, &PocketRimVBO);
        glDeleteBuffers(1, &PocketRimEBO);
    }
}

void Table::InitMesh()
{
    GenerateSurfaceMesh();
    GenerateCushionMesh();
    GenerateFrameMesh();
    GeneratePocketMesh();
    GeneratePocketRimMesh();
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
    float cw = CushionWidth;
    float frameWidth = 0.1f; // must match GenerateFrameMesh

    // Extend surface to cover entire area under cushions and frame
    // so there are no transparent gaps at pockets or table edges
    float ex = hw + cw + frameWidth;
    float ez = hl + cw + frameWidth;

    // Playing surface is a flat quad at Y = 0
    Vertex v0, v1, v2, v3;

    // Bottom-left (looking down from +Y)
    v0.position[0] = -ex; v0.position[1] = 0.0f; v0.position[2] = -ez;
    v0.normal[0] = 0.0f;  v0.normal[1] = 1.0f;   v0.normal[2] = 0.0f;
    v0.texCoord[0] = 0.0f; v0.texCoord[1] = 0.0f;

    // Bottom-right
    v1.position[0] = ex;  v1.position[1] = 0.0f; v1.position[2] = -ez;
    v1.normal[0] = 0.0f;  v1.normal[1] = 1.0f;   v1.normal[2] = 0.0f;
    v1.texCoord[0] = 1.0f; v1.texCoord[1] = 0.0f;

    // Top-right
    v2.position[0] = ex;  v2.position[1] = 0.0f; v2.position[2] = ez;
    v2.normal[0] = 0.0f;  v2.normal[1] = 1.0f;   v2.normal[2] = 0.0f;
    v2.texCoord[0] = 1.0f; v2.texCoord[1] = 1.0f;

    // Top-left
    v3.position[0] = -ex; v3.position[1] = 0.0f; v3.position[2] = ez;
    v3.normal[0] = 0.0f;  v3.normal[1] = 1.0f;   v3.normal[2] = 0.0f;
    v3.texCoord[0] = 0.0f; v3.texCoord[1] = 1.0f;

    mesh.vertices.push_back(v0);
    mesh.vertices.push_back(v1);
    mesh.vertices.push_back(v2);
    mesh.vertices.push_back(v3);

    // Wind CCW when viewed from above (+Y) so the surface is front-facing to the camera
    mesh.indices.push_back(0);
    mesh.indices.push_back(2);
    mesh.indices.push_back(1);
    mesh.indices.push_back(0);
    mesh.indices.push_back(3);
    mesh.indices.push_back(2);

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
    auto addBox = [&mesh](float minX, float minY, float minZ, float maxX, float maxY, float maxZ, int skipFaces = 0)
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
            { {4, 5, 6, 7}, Vec3(0, 0, 1) },   // 0: Front (+Z)
            { {1, 0, 3, 2}, Vec3(0, 0, -1) },  // 1: Back (-Z)
            { {5, 1, 2, 6}, Vec3(1, 0, 0) },   // 2: Right (+X)
            { {0, 4, 7, 3}, Vec3(-1, 0, 0) },  // 3: Left (-X)
            { {7, 6, 2, 3}, Vec3(0, 1, 0) },   // 4: Top (+Y)
            { {0, 1, 5, 4}, Vec3(0, -1, 0) }   // 5: Bottom (-Y)
        };

        for (int f = 0; f < 6; f++)
        {
            if (skipFaces & (1 << f)) continue;

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

    // Cushions with gaps computed from pocket positions so rims connect flush.
    // Each gap starts at (pocket center + PocketRadius) along the cushion.
    float pr = PocketRadius;


    // Left cushion (-X side): 2 segments with gaps at corners + side pocket
    float lz1s = PocketPositions[0].z + pr;  // after back-left pocket
    float lz1e = PocketPositions[4].z - pr;  // before left side pocket
    float lz2s = PocketPositions[4].z + pr;  // after left side pocket
    float lz2e = PocketPositions[2].z - pr;  // before front-left pocket
    addBox(-hw - cw, 0, lz1s, -hw, ch, lz1e, 32 | 8);
    addBox(-hw - cw, 0, lz2s, -hw, ch, lz2e, 32 | 8);

    // Right cushion (+X side)
    float rz1s = PocketPositions[1].z + pr;
    float rz1e = PocketPositions[5].z - pr;
    float rz2s = PocketPositions[5].z + pr;
    float rz2e = PocketPositions[3].z - pr;
    addBox(hw, 0, rz1s, hw + cw, ch, rz1e, 32 | 4);
    addBox(hw, 0, rz2s, hw + cw, ch, rz2e, 32 | 4);

    // Back cushion (-Z side): one segment between corner pockets
    float bxs = PocketPositions[0].x + pr;
    float bxe = PocketPositions[1].x - pr;
    addBox(bxs, 0, -hl - cw, bxe, ch, -hl, 32 | 2);

    // Front cushion (+Z side)
    float fxs = PocketPositions[2].x + pr;
    float fxe = PocketPositions[3].x - pr;
    addBox(fxs, 0, hl, fxe, ch, hl + cw, 32 | 1);

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
    auto addBox = [&mesh](float minX, float minY, float minZ, float maxX, float maxY, float maxZ, int skipFaces = 0)
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
            if (skipFaces & (1 << f)) continue;

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


    // Left frame (Z between back and front frames, skip front+back faces at junctions)
    addBox(-outerX, frameBottom, -hl - cw, -hw - cw, frameTop, hl + cw, 1 | 2);

    // Right frame
    addBox(hw + cw, frameBottom, -hl - cw, outerX, frameTop, hl + cw, 1 | 2);

    // Back frame (full X width, fills corners)
    addBox(-outerX, frameBottom, -outerZ, outerX, frameTop, -hl - cw);

    // Front frame (full X width, fills corners)
    addBox(-outerX, frameBottom, hl + cw, outerX, frameTop, outerZ);

    UploadMesh(mesh, FrameVAO, FrameVBO, FrameEBO, FrameIndexCount);
}

void Table::GeneratePocketMesh()
{
    MeshData mesh = GenerateDiscMesh(PocketRadius, 48);
    UploadMesh(mesh, PocketVAO, PocketVBO, PocketEBO, PocketIndexCount);
}

void Table::GeneratePocketRimMesh()
{
    MeshData mesh;

    float hw = Width / 2.0f;
    float hl = Length / 2.0f;
    float rimHeight = CushionHeight;
    int segments = 48;

    // Arc definitions per pocket: startAngle, endAngle
    // Angle 0 = +X direction, PI/2 = +Z direction
    struct PocketArc { float startAngle; float endAngle; };
    PocketArc arcs[NUM_POCKETS] = {
        { 0.0f,            PI / 2.0f },           // 0: back-left  (-hw, -hl) - quarter facing +X/+Z
        { PI / 2.0f,       PI },                   // 1: back-right (+hw, -hl) - quarter facing -X/+Z
        { 3.0f * PI / 2.0f, 2.0f * PI },           // 2: front-left (-hw, +hl) - quarter facing +X/-Z
        { PI,              3.0f * PI / 2.0f },      // 3: front-right(+hw, +hl) - quarter facing -X/-Z
        { -PI / 2.0f,     PI / 2.0f },             // 4: left side  (-hw, 0)   - semicircle facing +X
        { PI / 2.0f,      3.0f * PI / 2.0f }       // 5: right side (+hw, 0)   - semicircle facing -X
    };

    for (int p = 0; p < NUM_POCKETS; p++)
    {
        Vec3 center = PocketPositions[p];
        float startA = arcs[p].startAngle;
        float endA = arcs[p].endAngle;
        float arcLength = endA - startA;
        // Quarter circles for corners, semicircles for side pockets
        int numSegs = (p < 4) ? segments / 4 : segments / 2;

        for (int i = 0; i < numSegs; i++)
        {
            float t0 = (float)i / numSegs;
            float t1 = (float)(i + 1) / numSegs;
            float angle0 = startA + t0 * arcLength;
            float angle1 = startA + t1 * arcLength;

            float x0 = center.x + PocketRadius * cosf(angle0);
            float z0 = center.z + PocketRadius * sinf(angle0);
            float x1 = center.x + PocketRadius * cosf(angle1);
            float z1 = center.z + PocketRadius * sinf(angle1);

            // Outward-facing normals (away from pocket center, toward playing area)
            float nx0 = cosf(angle0);
            float nz0 = sinf(angle0);
            float nx1 = cosf(angle1);
            float nz1 = sinf(angle1);

            unsigned int base = (unsigned int)mesh.vertices.size();

            // v0: bottom at angle0
            Vertex v0;
            v0.position[0] = x0; v0.position[1] = 0.0f; v0.position[2] = z0;
            v0.normal[0] = nx0;  v0.normal[1] = 0.0f;   v0.normal[2] = nz0;
            v0.texCoord[0] = t0; v0.texCoord[1] = 0.0f;

            // v1: bottom at angle1
            Vertex v1;
            v1.position[0] = x1; v1.position[1] = 0.0f; v1.position[2] = z1;
            v1.normal[0] = nx1;  v1.normal[1] = 0.0f;   v1.normal[2] = nz1;
            v1.texCoord[0] = t1; v1.texCoord[1] = 0.0f;

            // v2: top at angle1
            Vertex v2;
            v2.position[0] = x1; v2.position[1] = rimHeight; v2.position[2] = z1;
            v2.normal[0] = nx1;  v2.normal[1] = 0.0f;       v2.normal[2] = nz1;
            v2.texCoord[0] = t1; v2.texCoord[1] = 1.0f;

            // v3: top at angle0
            Vertex v3;
            v3.position[0] = x0; v3.position[1] = rimHeight; v3.position[2] = z0;
            v3.normal[0] = nx0;  v3.normal[1] = 0.0f;       v3.normal[2] = nz0;
            v3.texCoord[0] = t0; v3.texCoord[1] = 1.0f;

            mesh.vertices.push_back(v0);
            mesh.vertices.push_back(v1);
            mesh.vertices.push_back(v2);
            mesh.vertices.push_back(v3);

            // Two triangles per quad (wound CCW when viewed from outside the pocket)
            mesh.indices.push_back(base + 0);
            mesh.indices.push_back(base + 2);
            mesh.indices.push_back(base + 1);
            mesh.indices.push_back(base + 0);
            mesh.indices.push_back(base + 3);
            mesh.indices.push_back(base + 2);
        }
    }

    UploadMesh(mesh, PocketRimVAO, PocketRimVBO, PocketRimEBO, PocketRimIndexCount);
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
