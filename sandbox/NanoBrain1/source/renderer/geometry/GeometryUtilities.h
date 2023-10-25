#pragma once

#include "../Shared.h"
#include "../handlers/Handlers.h"

// Type alias for better readability
using Eigen::Vector3f;
using rapidobj::Attributes;
using rapidobj::Index;
using rapidobj::Material;
using rapidobj::MaterialLibrary;
using rapidobj::Mesh;

// Define constants for multi-threading and floating point operations
#define GRAIN_SIZE 1024
#if defined(_WIN32)
#define RCPOVERFLOW_FLT 2.93873587705571876e-39f
#define RCPOVERFLOW_DBL 5.56268464626800345e-309
#else
#define RCPOVERFLOW_FLT 0x1p-128f
#define RCPOVERFLOW_DBL 0x1p-1024
#endif

#if defined(SINGLE_PRECISION)
#define RCPOVERFLOW RCPOVERFLOW_FLT
#else
#define RCPOVERFLOW RCPOVERFLOW_DBL
#endif

// Fast acos implementation from Instant Meshes
inline float fast_acos (float x)
{
    float negate = float (x < 0.0f);
    x = std::abs (x);
    float ret = -0.0187293f;
    ret *= x;
    ret = ret + 0.0742610f;
    ret *= x;
    ret = ret - 0.2121144f;
    ret *= x;
    ret = ret + 1.5707288f;
    ret = ret * std::sqrt (1.0f - x);
    ret = ret - 2.0f * negate * ret;
    return negate * (float)M_PI + ret;
}

// Generate normals for mesh
// replaced TBB with BS_thread_pool, math from Instant Meshes https://github.com/wjakob/instant-meshes
void generate_normals (const MatrixXu& F, const MatrixXf& V, MatrixXf& N, MatrixXf& FN)
{
    ScopedStopWatch sw ("GENERATE NORMALS"); // Start timer

    std::atomic<uint32_t> badFaces (0); // Counter for degenerate faces

    N.resize (V.rows(), V.cols()); // Prepare vertex normal matrix
    N.setZero();

    FN.resize (F.rows(), F.cols()); // Prepare face normal matrix
    FN.setZero();

    BS::thread_pool pool; // Initialize thread pool

    // Multi-threaded computation of face and vertex normals
    auto map = [&] (const uint32_t start, const uint32_t end)
    {
        for (uint32_t f = start; f < end; ++f)
        {
            Vector3f fn = Vector3f::Zero();
            for (int i = 0; i < 3; ++i)
            {
                Vector3f v0 = V.col (F (i, f)),
                         v1 = V.col (F ((i + 1) % 3, f)),
                         v2 = V.col (F ((i + 2) % 3, f)),
                         d0 = v1 - v0,
                         d1 = v2 - v0;

                if (i == 0)
                {
                    fn = d0.cross (d1);
                    Float norm = fn.norm();
                    if (norm < RCPOVERFLOW)
                    {
                        badFaces++;
                        break;
                    }
                    FN.col (f) = fn.normalized();
                    fn /= norm;
                }

                Float angle = fast_acos (d0.dot (d1) / std::sqrt (d0.squaredNorm() * d1.squaredNorm()));
                for (uint32_t k = 0; k < 3; ++k)
                    mace::atomicAdd (&N.coeffRef (k, F (i, f)), fn[k] * angle);
            }
        }
    };

    pool.push_loop (0u, (uint32_t)F.cols(), map, GRAIN_SIZE); // Execute in parallel

    // must wait here because the normalize task depends on this task being completed
    pool.wait_for_tasks();

    // Normalize the vertex normals
    pool.push_loop (0u, (uint32_t)V.cols(),
                    [&] (const uint32_t start, const uint32_t end)
                    {
                        for (uint32_t i = start; i < end; ++i)
                        {
                            Float norm = N.col (i).norm();
                            if (norm < RCPOVERFLOW)
                            {
                                N.col (i) = Vector3f::UnitX();
                            }
                            else
                            {
                                N.col (i) /= norm;
                            }
                        }
                    });

    pool.wait_for_tasks(); 
}

// Rapid Obj helpers
inline void ReportError (const rapidobj::Error& error)
{
    LOG (CRITICAL) << error.code.message();
    if (!error.line.empty())
    {
        LOG (DBUG) << "On line " << error.line_num << ": \"" << error.line;
    }
}

inline std::pair<uint32_t, uint32_t> getTotalVertexAndTriangleCounts (const rapidobj::Result& result, std::unordered_set<int>& unique_vertices,
                                                                      std::unordered_set<int>& unique_uv)
{
    // WTF is this the only way to get the vertex count?
    uint32_t vertexCount{};
    uint32_t triangleCount{};
    uint32_t uvCount{};
    for (auto& s : result.shapes)
    {
        triangleCount += s.mesh.num_face_vertices.size();
        for (const auto& index : s.mesh.indices)
        {
            if (unique_vertices.insert (index.position_index).second)
            {
                ++vertexCount;
            }

            if (unique_uv.insert (index.texcoord_index).second)
            {
                ++uvCount;
            }
        }
    }

    return std::make_pair (vertexCount, triangleCount);
}

inline void getMaterialIdList (const rapidobj::Result& result, std::vector<uint8_t>& materialIDs)
{
    uint32_t index = 0;

    for (auto& s : result.shapes)
    {
        for (auto& id : s.mesh.material_ids)
        {
            if (index < materialIDs.size())
                materialIDs[index] = static_cast<uint8_t> (id);
            ++index;
        }
    }
}

inline void getTriangleIndices (const rapidobj::Result& result, std::vector<int>& triangleIndices)
{
    const Attributes& attributes = result.attributes;

    for (auto& s : result.shapes)
    {
        for (const auto& index : s.mesh.indices)
        {
            triangleIndices.push_back (index.position_index);
        }
    }
}

inline void getVertexPositions (const rapidobj::Result& result, MatrixXf& V, std::unordered_set<int>& unique_vertices_indices)
{
    const Attributes& attributes = result.attributes;
    for (const auto& position_index : unique_vertices_indices)
    {
        float x = attributes.positions[position_index * 3];
        float y = attributes.positions[position_index * 3 + 1];
        float z = attributes.positions[position_index * 3 + 2];

        V.col (position_index) = Vector3f (x, y, z);
    }
}

inline void getUVs (const rapidobj::Result& result, MatrixXf& UV, std::unordered_set<int>& unique_vertices_indices)
{
    const Attributes& attributes = result.attributes;
    for (const auto& position_index : unique_vertices_indices)
    {
        float x = attributes.texcoords[position_index * 2];
        float y = attributes.texcoords[position_index * 2 + 1];

        UV.col (position_index) = Eigen::Vector2f (x, y);
    }
}

inline void normalizeSize (const AlignedBox3f& modelBound, float& scale)
{
    Eigen::Vector3f edges = modelBound.max() - modelBound.min();
    float maxEdge = std::max (edges.x(), std::max (edges.y(), edges.z()));
    scale = 1.0f / maxEdge; // max
}

inline void centerVertices (MatrixXf& V, const AlignedBox3f& modelBound, float scale)
{
    int pointCount = V.cols();
    Vector3f center = modelBound.center();
    for (int i = 0; i < pointCount; i++)
    {
        Vector3f pnt = V.col (i);
        pnt -= center;
        pnt *= scale;
        V.col (i) = pnt;
    }
}