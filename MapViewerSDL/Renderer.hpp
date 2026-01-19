#pragma once

#include "Camera.hpp"
#include "gl_core_3_3.hpp"
#include "utility/Vector.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <unordered_set>
#include <vector>

class Renderer
{
public:
    enum Geometry
    {
        TerrainGeometry,
        LiquidGeometry,
        WmoGeometry,
        DoodadGeometry,
        NavMeshGeometry,
        SphereGeometry,
        LineGeometry,
        ArrowGeometry,
        GameObjectGeometry,
        NumGeometryBuffers,
    };

    enum GeometryFlags
    {
        TerrainGeometryFlag = 1 << TerrainGeometry,
        LiquidGeometryFlag = 1 << LiquidGeometry,
        WmoGeometryFlag = 1 << WmoGeometry,
        DoodadGeometryFlag = 1 << DoodadGeometry,
        NavMeshGeometryFlag = 1 << NavMeshGeometry,
        SphereGeometryFlag = 1 << SphereGeometry,
        LineGeometryFlag = 1 << LineGeometry,
        ArrowGeometryFlag = 1 << ArrowGeometry,
        GameObjectGeometryFlag = 1 << GameObjectGeometry,

        CollidableGeometryFlag =
            TerrainGeometryFlag | WmoGeometryFlag | DoodadGeometryFlag,
    };

private:
    static constexpr glm::vec4 LiquidColor{0.25f, 0.28f, 0.9f, 0.5f};
    static constexpr glm::vec4 WmoColor{1.f, 0.95f, 0.f, 1.f};
    static constexpr glm::vec4 DoodadColor{1.f, 0.f, 0.f, 1.f};
    static constexpr glm::vec4 BackgroundColor{0.6f, 0.55f, 0.55f, 1.f};
    static constexpr glm::vec4 SphereColor{1.f, 0.5f, 0.25f, 0.75f};
    static constexpr glm::vec4 LineColor{0.5f, 0.25f, 0.0f, 1.f};
    static constexpr glm::vec4 ArrowColor{0.5f, 0.25f, 0.0f, 1.f};
    static constexpr glm::vec4 GameObjectColor{0.8f, 0.5f, 0.1f, 1.f};
    static constexpr glm::vec4 MeshColor{1.f, 1.f, 1.f, 0.75f};
    static constexpr glm::vec4 MeshSteepColor{0.3f, 0.3f, 0.3f, 0.75f};
    static constexpr glm::vec4 TerrainColor{0.5f, 0.8f, 0.5f, 1.0f};

    struct ColoredVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 color;
    };

    struct GeometryBuffer
    {
        std::uint32_t UserParameter;

        std::vector<ColoredVertex> vertices;
        std::vector<unsigned int> indices;

        GLuint vao;
        GLuint vbo;
        GLuint ebo;
        bool uploaded;

        GeometryBuffer()
            : UserParameter(0), vao(0), vbo(0), ebo(0), uploaded(false)
        {
        }
    };

    GLuint m_shaderProgram;
    GLint m_mvpLocation;

    std::vector<GeometryBuffer> m_buffers[NumGeometryBuffers];

    std::unordered_set<unsigned int> m_wmos;
    std::unordered_set<unsigned int> m_doodads;

    bool m_renderADT;
    bool m_renderLiquid;
    bool m_renderWMO;
    bool m_renderDoodad;
    bool m_renderMesh;

    bool m_wireframeEnabled;

    void InsertBuffer(std::vector<GeometryBuffer>& buffer, const glm::vec4& color,
                      const std::vector<math::Vertex>& vertices,
                      const std::vector<int>& indices,
                      std::uint32_t userParam = 0, bool genNormals = true);

    void UploadBuffer(GeometryBuffer& buffer);
    void RenderBuffer(const GeometryBuffer& buffer, const glm::mat4& mvp);

    void InitializeShaders();
    void CleanupBuffer(GeometryBuffer& buffer);

    glm::vec3 ToGlm(const math::Vertex& v) const { return glm::vec3(v.X, v.Y, v.Z); }

public:
    Renderer();
    ~Renderer();

    void Initialize();
    void Cleanup();

    void ClearBuffers(Geometry type);
    void ClearBuffers();
    void ClearSprites();
    void ClearGameObjects();

    void AddTerrain(const std::vector<math::Vertex>& vertices,
                    const std::vector<int>& indices, std::uint32_t areaId);
    void AddLiquid(const std::vector<math::Vertex>& vertices,
                   const std::vector<int>& indices);
    void AddWmo(unsigned int id, const std::vector<math::Vertex>& vertices,
                const std::vector<int>& indices);
    void AddDoodad(unsigned int id, const std::vector<math::Vertex>& vertices,
                   const std::vector<int>& indices);
    void AddMesh(const std::vector<math::Vertex>& vertices,
                 const std::vector<int>& indices, bool steep);
    void AddLines(const std::vector<math::Vertex>& vertices,
                  const std::vector<int>& indices);
    void AddSphere(const math::Vertex& position, float size,
                   int recursionLevel = 2);
    void AddArrows(const math::Vertex& start, const math::Vertex& end,
                   float step);
    void AddPath(const std::vector<math::Vertex>& path);
    void AddGameObject(const std::vector<math::Vertex>& vertices,
                       const std::vector<int>& indices);

    bool HasWmo(unsigned int id) const;
    bool HasDoodad(unsigned int id) const;

    void Render(const Camera& camera, int width, int height);

    void SetWireframe(bool enabled);
    void SetRenderADT(bool enabled) { m_renderADT = enabled; }
    void SetRenderLiquid(bool enabled) { m_renderLiquid = enabled; }
    void SetRenderWMO(bool enabled) { m_renderWMO = enabled; }
    void SetRenderDoodad(bool enabled) { m_renderDoodad = enabled; }
    void SetRenderMesh(bool enabled) { m_renderMesh = enabled; }

    bool HitTest(const Camera& camera, int x, int y, std::uint32_t geometryFlags,
                 math::Vertex& out, std::uint32_t& param) const;
};
