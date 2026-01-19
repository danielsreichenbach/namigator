#include "Renderer.hpp"
#include "SphereMesh.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Simple vertex shader
static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;

out vec4 vertexColor;
out vec3 fragNormal;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
    vertexColor = aColor;
    fragNormal = aNormal;
}
)";

// Simple fragment shader with basic lighting
static const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec4 vertexColor;
in vec3 fragNormal;

void main()
{
    // Simple directional lighting
    vec3 lightDir = normalize(vec3(0.5, 0.5, 1.0));
    float ambient = 0.4;
    float diffuse = max(dot(normalize(fragNormal), lightDir), 0.0) * 0.6;
    float lighting = ambient + diffuse;

    FragColor = vec4(vertexColor.rgb * lighting, vertexColor.a);
}
)";

Renderer::Renderer()
    : m_shaderProgram(0), m_mvpLocation(0), m_renderADT(true),
      m_renderLiquid(true), m_renderWMO(true), m_renderDoodad(true),
      m_renderMesh(true), m_wireframeEnabled(false)
{
}

Renderer::~Renderer()
{
    Cleanup();
}

void Renderer::Initialize()
{
    // Load OpenGL 3.3 function pointers
    if (!LoadGL33Functions())
    {
        std::cerr << "Failed to load OpenGL 3.3 functions" << std::endl;
        return;
    }

    // Print OpenGL version
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);

    std::cout << "OpenGL Version: " << version << std::endl;
    std::cout << "OpenGL Vendor: " << vendor << std::endl;
    std::cout << "OpenGL Renderer: " << renderer << std::endl;

    InitializeShaders();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable blending for transparent objects
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable back-face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    std::cout << "Renderer initialized successfully" << std::endl;
}

void Renderer::InitializeShaders()
{
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check vertex shader compilation
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    // Link shaders
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);

    // Check linking
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Get uniform location
    m_mvpLocation = glGetUniformLocation(m_shaderProgram, "mvp");
}

void Renderer::Cleanup()
{
    // Clean up all buffers
    for (int i = 0; i < NumGeometryBuffers; ++i)
    {
        for (auto& buffer : m_buffers[i])
            CleanupBuffer(buffer);
        m_buffers[i].clear();
    }

    if (m_shaderProgram)
        glDeleteProgram(m_shaderProgram);
}

void Renderer::CleanupBuffer(GeometryBuffer& buffer)
{
    if (buffer.vao)
        glDeleteVertexArrays(1, &buffer.vao);
    if (buffer.vbo)
        glDeleteBuffers(1, &buffer.vbo);
    if (buffer.ebo)
        glDeleteBuffers(1, &buffer.ebo);

    buffer.vao = buffer.vbo = buffer.ebo = 0;
    buffer.uploaded = false;
}

void Renderer::InsertBuffer(std::vector<GeometryBuffer>& bufferList,
                            const glm::vec4& color,
                            const std::vector<math::Vertex>& vertices,
                            const std::vector<int>& indices,
                            std::uint32_t userParam, bool genNormals)
{
    GeometryBuffer buffer;
    buffer.UserParameter = userParam;

    // Convert vertices and generate normals if requested
    buffer.vertices.reserve(vertices.size());
    for (const auto& v : vertices)
    {
        ColoredVertex cv;
        cv.position = ToGlm(v);
        cv.normal = glm::vec3(0.0f, 0.0f, 1.0f); // Default normal
        cv.color = color;
        buffer.vertices.push_back(cv);
    }

    // Convert indices
    buffer.indices.reserve(indices.size());
    for (const auto& idx : indices)
        buffer.indices.push_back(static_cast<unsigned int>(idx));

    // Generate normals from triangles if requested
    if (genNormals && buffer.indices.size() >= 3)
    {
        for (size_t i = 0; i < buffer.indices.size(); i += 3)
        {
            if (i + 2 >= buffer.indices.size())
                break;

            auto idx0 = buffer.indices[i];
            auto idx1 = buffer.indices[i + 1];
            auto idx2 = buffer.indices[i + 2];

            if (idx0 >= buffer.vertices.size() || idx1 >= buffer.vertices.size() ||
                idx2 >= buffer.vertices.size())
                continue;

            glm::vec3& p0 = buffer.vertices[idx0].position;
            glm::vec3& p1 = buffer.vertices[idx1].position;
            glm::vec3& p2 = buffer.vertices[idx2].position;

            glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));

            buffer.vertices[idx0].normal = normal;
            buffer.vertices[idx1].normal = normal;
            buffer.vertices[idx2].normal = normal;
        }
    }

    bufferList.push_back(std::move(buffer));
}

void Renderer::UploadBuffer(GeometryBuffer& buffer)
{
    if (buffer.uploaded || buffer.vertices.empty())
        return;

    // Generate VAO
    glGenVertexArrays(1, &buffer.vao);
    glBindVertexArray(buffer.vao);

    // Generate VBO
    glGenBuffers(1, &buffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, buffer.vertices.size() * sizeof(ColoredVertex),
                 buffer.vertices.data(), GL_STATIC_DRAW);

    // Generate EBO
    if (!buffer.indices.empty())
    {
        glGenBuffers(1, &buffer.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     buffer.indices.size() * sizeof(unsigned int),
                     buffer.indices.data(), GL_STATIC_DRAW);
    }

    // Set vertex attributes
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex),
                          (void*)offsetof(ColoredVertex, position));
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex),
                          (void*)offsetof(ColoredVertex, normal));
    glEnableVertexAttribArray(1);

    // Color
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex),
                          (void*)offsetof(ColoredVertex, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    buffer.uploaded = true;
}

void Renderer::RenderBuffer(const GeometryBuffer& buffer, const glm::mat4& mvp)
{
    if (!buffer.uploaded || buffer.vertices.empty())
        return;

    glUniformMatrix4fv(m_mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

    glBindVertexArray(buffer.vao);

    if (!buffer.indices.empty())
    {
        glDrawElements(GL_TRIANGLES, buffer.indices.size(), GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, buffer.vertices.size());
    }

    glBindVertexArray(0);
}

void Renderer::Render(const Camera& camera, int width, int height)
{
    // Clear screen
    glClearColor(BackgroundColor.r, BackgroundColor.g, BackgroundColor.b,
                 BackgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set viewport
    glViewport(0, 0, width, height);

    // Use shader program
    glUseProgram(m_shaderProgram);

    // Calculate MVP matrix
    glm::mat4 mvp = camera.GetProjMatrix() * camera.GetViewMatrix();

    // Set wireframe mode if enabled
    if (m_wireframeEnabled)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Upload and render each geometry type
    auto renderGeometry = [&](Geometry type, bool enabled)
    {
        if (!enabled)
            return;

        for (auto& buffer : m_buffers[type])
        {
            if (!buffer.uploaded)
                UploadBuffer(buffer);
            RenderBuffer(buffer, mvp);
        }
    };

    // Render opaque geometry first
    glDepthMask(GL_TRUE);
    renderGeometry(TerrainGeometry, m_renderADT);
    renderGeometry(WmoGeometry, m_renderWMO);
    renderGeometry(DoodadGeometry, m_renderDoodad);
    renderGeometry(GameObjectGeometry, true);

    // Render transparent geometry last
    glDepthMask(GL_FALSE);
    renderGeometry(LiquidGeometry, m_renderLiquid);
    renderGeometry(NavMeshGeometry, m_renderMesh);
    glDepthMask(GL_TRUE);

    // Render debug visualization (always visible)
    renderGeometry(SphereGeometry, true);
    renderGeometry(LineGeometry, true);
    renderGeometry(ArrowGeometry, true);

    glUseProgram(0);
}

void Renderer::ClearBuffers(Geometry type)
{
    for (auto& buffer : m_buffers[type])
        CleanupBuffer(buffer);
    m_buffers[type].clear();
}

void Renderer::ClearBuffers()
{
    for (int i = 0; i < NumGeometryBuffers; ++i)
        ClearBuffers(static_cast<Geometry>(i));

    m_wmos.clear();
    m_doodads.clear();
}

void Renderer::ClearSprites()
{
    ClearBuffers(SphereGeometry);
    ClearBuffers(LineGeometry);
    ClearBuffers(ArrowGeometry);
}

void Renderer::ClearGameObjects()
{
    ClearBuffers(GameObjectGeometry);
}

void Renderer::AddTerrain(const std::vector<math::Vertex>& vertices,
                          const std::vector<int>& indices, std::uint32_t areaId)
{
    InsertBuffer(m_buffers[TerrainGeometry], TerrainColor, vertices, indices,
                 areaId);
}

void Renderer::AddLiquid(const std::vector<math::Vertex>& vertices,
                         const std::vector<int>& indices)
{
    InsertBuffer(m_buffers[LiquidGeometry], LiquidColor, vertices, indices);
}

void Renderer::AddWmo(unsigned int id, const std::vector<math::Vertex>& vertices,
                      const std::vector<int>& indices)
{
    m_wmos.insert(id);
    InsertBuffer(m_buffers[WmoGeometry], WmoColor, vertices, indices, id);
}

void Renderer::AddDoodad(unsigned int id,
                         const std::vector<math::Vertex>& vertices,
                         const std::vector<int>& indices)
{
    m_doodads.insert(id);
    InsertBuffer(m_buffers[DoodadGeometry], DoodadColor, vertices, indices, id);
}

void Renderer::AddMesh(const std::vector<math::Vertex>& vertices,
                       const std::vector<int>& indices, bool steep)
{
    const glm::vec4& color = steep ? MeshSteepColor : MeshColor;
    InsertBuffer(m_buffers[NavMeshGeometry], color, vertices, indices);
}

void Renderer::AddLines(const std::vector<math::Vertex>& vertices,
                        const std::vector<int>& indices)
{
    InsertBuffer(m_buffers[LineGeometry], LineColor, vertices, indices, 0, false);
}

void Renderer::AddSphere(const math::Vertex& position, float size,
                         int recursionLevel)
{
    auto [vertices, indices] = GenerateSphereMesh(position, size, recursionLevel);
    InsertBuffer(m_buffers[SphereGeometry], SphereColor, vertices, indices);
}

void Renderer::AddArrows(const math::Vertex& start, const math::Vertex& end,
                         float step)
{
    // Simple arrow visualization as lines
    std::vector<math::Vertex> vertices = {start, end};
    std::vector<int> indices = {0, 1};
    InsertBuffer(m_buffers[ArrowGeometry], ArrowColor, vertices, indices, 0, false);
}

void Renderer::AddPath(const std::vector<math::Vertex>& path)
{
    if (path.size() < 2)
        return;

    // Add spheres at each waypoint
    for (const auto& point : path)
        AddSphere(point, 1.5f, 1);

    // Add lines connecting waypoints
    std::vector<math::Vertex> lineVertices;
    std::vector<int> lineIndices;

    for (size_t i = 0; i + 1 < path.size(); ++i)
    {
        lineVertices.push_back(path[i]);
        lineVertices.push_back(path[i + 1]);
        lineIndices.push_back(i * 2);
        lineIndices.push_back(i * 2 + 1);
    }

    AddLines(lineVertices, lineIndices);
}

void Renderer::AddGameObject(const std::vector<math::Vertex>& vertices,
                             const std::vector<int>& indices)
{
    InsertBuffer(m_buffers[GameObjectGeometry], GameObjectColor, vertices, indices);
}

bool Renderer::HasWmo(unsigned int id) const
{
    return m_wmos.find(id) != m_wmos.end();
}

bool Renderer::HasDoodad(unsigned int id) const
{
    return m_doodads.find(id) != m_doodads.end();
}

void Renderer::SetWireframe(bool enabled)
{
    m_wireframeEnabled = enabled;
}

bool Renderer::HitTest(const Camera& camera, int x, int y,
                       std::uint32_t geometryFlags, math::Vertex& out,
                       std::uint32_t& param) const
{
    // Get ray from camera through screen point
    glm::vec3 rayOrigin, rayDir;
    camera.GetPickRay(x, y, rayOrigin, rayDir);

    float closestDist = std::numeric_limits<float>::max();
    bool found = false;

    // Check each enabled geometry type
    for (int geomType = 0; geomType < NumGeometryBuffers; ++geomType)
    {
        if (!(geometryFlags & (1 << geomType)))
            continue;

        for (const auto& buffer : m_buffers[geomType])
        {
            // Test each triangle
            for (size_t i = 0; i + 2 < buffer.indices.size(); i += 3)
            {
                const auto& v0 = buffer.vertices[buffer.indices[i]].position;
                const auto& v1 = buffer.vertices[buffer.indices[i + 1]].position;
                const auto& v2 = buffer.vertices[buffer.indices[i + 2]].position;

                // Ray-triangle intersection test (Möller-Trumbore algorithm)
                glm::vec3 edge1 = v1 - v0;
                glm::vec3 edge2 = v2 - v0;
                glm::vec3 h = glm::cross(rayDir, edge2);
                float a = glm::dot(edge1, h);

                if (a > -0.00001f && a < 0.00001f)
                    continue;

                float f = 1.0f / a;
                glm::vec3 s = rayOrigin - v0;
                float u = f * glm::dot(s, h);

                if (u < 0.0f || u > 1.0f)
                    continue;

                glm::vec3 q = glm::cross(s, edge1);
                float v = f * glm::dot(rayDir, q);

                if (v < 0.0f || u + v > 1.0f)
                    continue;

                float t = f * glm::dot(edge2, q);

                if (t > 0.00001f && t < closestDist)
                {
                    closestDist = t;
                    glm::vec3 hitPoint = rayOrigin + rayDir * t;
                    out = math::Vertex(hitPoint.x, hitPoint.y, hitPoint.z);
                    param = buffer.UserParameter;
                    found = true;
                }
            }
        }
    }

    return found;
}
