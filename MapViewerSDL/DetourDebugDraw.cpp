#include "DetourDebugDraw.hpp"

DetourDebugDraw::DetourDebugDraw(Renderer* renderer)
    : m_type(DU_DRAW_POINTS), m_size(1.f), m_steep(false), m_renderer(renderer)
{
}

void DetourDebugDraw::begin(duDebugDrawPrimitives prim, float size)
{
    m_type = prim;
    m_size = size;
    m_uniqueVertices.clear();
    m_vertices.clear();
    m_indices.clear();
    m_colors.clear();
}

void DetourDebugDraw::vertex(const float* pos, unsigned int color)
{
    vertex(pos[0], pos[1], pos[2], color);
}

void DetourDebugDraw::vertex(const float x, const float y, const float z,
                              unsigned int color)
{
    math::Vertex v(x, y, z);

    auto it = m_uniqueVertices.find(v);
    if (it != m_uniqueVertices.end())
    {
        m_indices.push_back(it->second);
    }
    else
    {
        int idx = static_cast<int>(m_vertices.size());
        m_vertices.push_back(v);
        m_indices.push_back(idx);
        m_uniqueVertices[v] = idx;
        m_colors.push_back(color);
    }
}

void DetourDebugDraw::vertex(const float* pos, unsigned int color, const float* /*uv*/)
{
    vertex(pos[0], pos[1], pos[2], color);
}

void DetourDebugDraw::vertex(const float x, const float y, const float z,
                              unsigned int color, const float /*u*/, const float /*v*/)
{
    vertex(x, y, z, color);
}

void DetourDebugDraw::end()
{
    // Skip lines with specific size (as in original)
    if (m_type == DU_DRAW_LINES && std::abs(m_size - IgnoreLineSize) < 0.001f)
        return;

    if (m_vertices.empty())
        return;

    switch (m_type)
    {
        case DU_DRAW_POINTS:
            // Draw points as small spheres
            for (const auto& v : m_vertices)
                m_renderer->AddSphere(v, m_size * 0.5f, 0);
            break;

        case DU_DRAW_LINES:
            m_renderer->AddLines(m_vertices, m_indices);
            break;

        case DU_DRAW_TRIS:
            // Add triangles to navigation mesh geometry
            m_renderer->AddMesh(m_vertices, m_indices, m_steep);
            break;

        case DU_DRAW_QUADS:
            // Convert quads to triangles
            if (m_indices.size() % 4 == 0)
            {
                std::vector<int> triIndices;
                for (size_t i = 0; i + 3 < m_indices.size(); i += 4)
                {
                    // First triangle
                    triIndices.push_back(m_indices[i]);
                    triIndices.push_back(m_indices[i + 1]);
                    triIndices.push_back(m_indices[i + 2]);

                    // Second triangle
                    triIndices.push_back(m_indices[i]);
                    triIndices.push_back(m_indices[i + 2]);
                    triIndices.push_back(m_indices[i + 3]);
                }
                m_renderer->AddMesh(m_vertices, triIndices, m_steep);
            }
            break;

        default:
            break;
    }
}
