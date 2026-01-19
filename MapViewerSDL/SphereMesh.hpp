#pragma once

#include "utility/Vector.hpp"
#include <cmath>
#include <tuple>
#include <vector>

// Generate an icosphere mesh for visualization
inline std::tuple<std::vector<math::Vertex>, std::vector<int>>
GenerateSphereMesh(const math::Vertex& center, float radius, int recursionLevel = 2)
{
    std::vector<math::Vertex> vertices;
    std::vector<int> indices;

    // Create icosahedron
    const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;

    // Add initial vertices (normalized icosahedron)
    auto addVertex = [&](float x, float y, float z)
    {
        float length = std::sqrt(x * x + y * y + z * z);
        vertices.push_back(math::Vertex(center.X + (x / length) * radius,
                                        center.Y + (y / length) * radius,
                                        center.Z + (z / length) * radius));
    };

    addVertex(-1, t, 0);
    addVertex(1, t, 0);
    addVertex(-1, -t, 0);
    addVertex(1, -t, 0);

    addVertex(0, -1, t);
    addVertex(0, 1, t);
    addVertex(0, -1, -t);
    addVertex(0, 1, -t);

    addVertex(t, 0, -1);
    addVertex(t, 0, 1);
    addVertex(-t, 0, -1);
    addVertex(-t, 0, 1);

    // Create initial triangles
    std::vector<int> initialIndices = {
        0, 11, 5,  0, 5, 1,   0, 1, 7,   0, 7, 10,  0, 10, 11,

        1, 5, 9,   5, 11, 4,  11, 10, 2, 10, 7, 6,  7, 1, 8,

        3, 9, 4,   3, 4, 2,   3, 2, 6,  3, 6, 8,   3, 8, 9,

        4, 9, 5,   2, 4, 11,  6, 2, 10, 8, 6, 7,   9, 8, 1};

    indices = initialIndices;

    // Subdivide triangles (simple subdivision for now)
    for (int level = 0; level < recursionLevel; ++level)
    {
        std::vector<int> newIndices;

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            int v0 = indices[i];
            int v1 = indices[i + 1];
            int v2 = indices[i + 2];

            // Get midpoints
            math::Vertex p0 = vertices[v0];
            math::Vertex p1 = vertices[v1];
            math::Vertex p2 = vertices[v2];

            math::Vertex m01((p0.X + p1.X) / 2, (p0.Y + p1.Y) / 2, (p0.Z + p1.Z) / 2);
            math::Vertex m12((p1.X + p2.X) / 2, (p1.Y + p2.Y) / 2, (p1.Z + p2.Z) / 2);
            math::Vertex m20((p2.X + p0.X) / 2, (p2.Y + p0.Y) / 2, (p2.Z + p0.Z) / 2);

            // Normalize to sphere surface
            auto normalize = [&](const math::Vertex& v)
            {
                float dx = v.X - center.X;
                float dy = v.Y - center.Y;
                float dz = v.Z - center.Z;
                float len = std::sqrt(dx * dx + dy * dy + dz * dz);
                return math::Vertex(center.X + (dx / len) * radius,
                                    center.Y + (dy / len) * radius,
                                    center.Z + (dz / len) * radius);
            };

            m01 = normalize(m01);
            m12 = normalize(m12);
            m20 = normalize(m20);

            int m01Idx = vertices.size();
            vertices.push_back(m01);
            int m12Idx = vertices.size();
            vertices.push_back(m12);
            int m20Idx = vertices.size();
            vertices.push_back(m20);

            // Create 4 new triangles
            newIndices.push_back(v0);
            newIndices.push_back(m01Idx);
            newIndices.push_back(m20Idx);

            newIndices.push_back(v1);
            newIndices.push_back(m12Idx);
            newIndices.push_back(m01Idx);

            newIndices.push_back(v2);
            newIndices.push_back(m20Idx);
            newIndices.push_back(m12Idx);

            newIndices.push_back(m01Idx);
            newIndices.push_back(m12Idx);
            newIndices.push_back(m20Idx);
        }

        indices = newIndices;
    }

    return {vertices, indices};
}
