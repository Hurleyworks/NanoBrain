#include "Jahley.h"
#include <nanothread/nanothread.h>
#include <rapidobj/rapidobj.hpp>
#include <rapidobj/include/serializer/serializer.hpp>

const std::string APP_NAME = "RapidObj";

using Eigen::Vector3f;
using rapidobj::Attributes;
using rapidobj::Index;
using rapidobj::Material;
using rapidobj::MaterialLibrary;
using rapidobj::Mesh;

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

// from Instant Meshes
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

// Payload structure to carry all necessary data
struct FaceCalcPayload
{
    const MatrixXu* F;
    const MatrixXf* V;
    MatrixXf* N;
    MatrixXf* FN;
    std::atomic<uint32_t>* badFaces;
};

// Static function for face calculation
static void faceCalcStatic (uint32_t index, void* data)
{
    auto* payload = static_cast<FaceCalcPayload*> (data);
    uint32_t f = index;
    Vector3f fn = Vector3f::Zero();

    const MatrixXu& F = *(payload->F);
    const MatrixXf& V = *(payload->V);
    MatrixXf& N = *(payload->N);
    MatrixXf& FN = *(payload->FN);
    std::atomic<uint32_t>& badFaces = *(payload->badFaces);

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
                badFaces++; /* degenerate */
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

// Payload structure for vertex calculation
struct VertexCalcPayload
{
    MatrixXf* N;
};

// Static function for vertex normal calculation
static void vertexCalcStatic (uint32_t index, void* data)
{
    auto* payload = static_cast<VertexCalcPayload*> (data);
    MatrixXf& N = *(payload->N);

    Vector3f n = N.col (index);
    Float norm = n.norm();
    if (norm < RCPOVERFLOW)
    {
        n = Vector3f::UnitX();
    }
    else
    {
        n /= norm;
    }
    N.col (index) = n; // Copy the modified vector back into the matrix
}

void generate_normals (const MatrixXu& F, const MatrixXf& V, MatrixXf& N, MatrixXf& FN, bool deterministic = false)
{
    ScopedStopWatch sw ("GENERATE NORMALS");

    // Create a worker per CPU thread
    Pool* const pool = pool_create (24);

    std::atomic<uint32_t> badFaces (0);

    // Initialize face and vertex normals to zero
    FN.setZero (F.rows(), F.cols());
    N.setZero (V.rows(), V.cols());

    // Calculate face normals first
    FaceCalcPayload facePayload = {&F, &V, &N, &FN, &badFaces};
    task_submit_and_wait (pool, F.cols(), faceCalcStatic, &facePayload);

    // Now calculate vertex normals
    VertexCalcPayload vertexPayload = {&N};
    task_submit_and_wait (pool, V.cols(), vertexCalcStatic, &vertexPayload);

    // Clean up used resources
    pool_destroy (pool);
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

std::pair<uint32_t, uint32_t> getTotalVertexAndTriangleCounts (const rapidobj::Result& result)
{
    // WTF is this the only way to get the vertex count?
    std::unordered_set<int> unique_vertices;
    uint32_t vertexCount{};
    uint32_t triangleCount{};
    for (auto& s : result.shapes)
    {
        triangleCount += s.mesh.num_face_vertices.size();
        for (const auto& index : s.mesh.indices)
        {
            if (unique_vertices.insert (index.position_index).second)
            {
                ++vertexCount;
            }
        }
    }

    return std::make_pair (vertexCount, triangleCount);
}

void getMaterialIdList (const rapidobj::Result& result, std::vector<int32_t>& materialIDs)
{
    uint32_t index = 0;

    for (auto& s : result.shapes)
    {
        for (const auto& id : s.mesh.material_ids)
        {
            if (index < materialIDs.size())
                materialIDs[index] = id;
            ++index;
        }
    }
}

void getTrianglesAndVertices (const rapidobj::Result& result, MatrixXf& V, std::vector<int>& triangleIndices)
{
    const Attributes& attributes = result.attributes;
    std::unordered_set<int> unique_vertices;

    for (auto& s : result.shapes)
    {
        for (const auto& index : s.mesh.indices)
        {
            int position_index = index.position_index;
            triangleIndices.push_back (position_index);

            if (unique_vertices.insert (position_index).second)
            {
                float x = attributes.positions[position_index * 3];
                float y = attributes.positions[position_index * 3 + 1];
                float z = attributes.positions[position_index * 3 + 2];

                V.col (position_index) = Vector3f (x, y, z);
            }
        }
    }
}

class Application : public Jahley::App
{
 public:
    Application() :
        Jahley::App()
    {
        try
        {
            // std::filesystem::path filePath = "C:/Users/Steve/Desktop/lw_cube.obj";
            // std::filesystem::path filePath = "E:/common_content/obj/2surf_cube.obj";
           // std::filesystem::path filePath = "E:/common_content/research_scenes/amazon_bistro/Exterior/static_exterior.obj";
            std::filesystem::path filePath = "E:/common_content/research_scenes/amazon_bistro/Interior/interior.obj";
            if (!std::filesystem::exists (filePath.generic_string()))
                throw std::runtime_error ("Load failed because file does not exist: " + filePath.generic_string());

            // MaterialLibrary Load policy Optional lets it load a obj file without a material and not crash
            MaterialLibrary ml = MaterialLibrary::Default (rapidobj::Load::Optional);

            rapidobj::Result result = rapidobj::ParseFile (filePath.generic_string(), ml);

            if (result.error)
            {
                LOG (CRITICAL) << result.error.code.message();

                throw std::runtime_error ("Load failed: " + filePath.generic_string());
            }

            rapidobj::Triangulate (result);

            if (result.error)
            {
                ReportError (result.error);
                throw std::runtime_error ("Triangulation failed");
            }

            uint32_t vertexCount, triangleCount;
            std::tie (vertexCount, triangleCount) = getTotalVertexAndTriangleCounts (result);

            // 1 materialID per triangle
            std::vector<int32_t> materialIDs (triangleCount);
            getMaterialIdList (result, materialIDs);

            // A flat array is a one-dimensional array where data is stored in a single, linear sequence
            std::vector<int32_t> triangleIndices;
            triangleIndices.reserve (triangleCount * 3);

            MatrixXf V;
            V.resize (3, vertexCount);
            getTrianglesAndVertices (result, V, triangleIndices);

            uint32_t elementCount = triangleIndices.size();
            uint32_t tris = elementCount / 3;
            assert (tris == triangleCount);

            MatrixXu F;
            F.resize (3, triangleCount);
            std::memcpy (F.data(), triangleIndices.data(), elementCount * sizeof (int));


            LOG (DBUG) << "This mesh has " << vertexCount << " vertices and " << triangleCount << " triangles";

            MatrixXf N;  // vertex normals
            MatrixXf FN; // face normals
            generate_normals (F, V, N, FN, false);

#if 0
            LOG (DBUG) << "---------------------";
            for (int i = 0; i < FN.cols(); ++i)
            {
                Vector3f n = FN.col (i);

                LOG (DBUG) << n.x() << ", " << n.y() << ", " << n.z();
            }
            LOG (DBUG) << "---------------------";
            for (int i = 0; i < N.cols(); ++i)
            {
                Vector3f n = N.col (i);

                LOG (DBUG) << n.x() << ", " << n.y() << ", " << n.z();
            }
#endif

            // 1 material ID per triangle
            assert (F.cols() == materialIDs.size());
        }
        catch (std::exception& e)
        {
            LOG (CRITICAL) << e.what();
        }
    }

    ~Application()
    {
    }

    void onCrash() override
    {
    }

 private:
};

#if 0
class Application : public Jahley::App
{
 public:
    Application() :
        Jahley::App()
    {
        try
        {
            // std::filesystem::path filePath = "C:/Users/Steve/Desktop/lw_cube.obj";
            std::filesystem::path filePath = "E:/common_content/obj/2surf_cube.obj";

            if (!std::filesystem::exists (filePath.generic_string()))
                throw std::runtime_error ("Load failed because file does not exist: " + filePath.generic_string());

            // MaterialLibrary Load policy Optional lets it load a obj file without a material and not crash
            MaterialLibrary ml = MaterialLibrary::Default (rapidobj::Load::Optional);

            rapidobj::Result result = rapidobj::ParseFile (filePath.generic_string(), ml);

            if (result.error)
            {
                LOG (CRITICAL) << result.error.code.message();

                throw std::runtime_error ("Load failed: " + filePath.generic_string());
            }

            rapidobj::Triangulate (result);

            if (result.error)
            {
                ReportError (result.error);
                throw std::runtime_error ("Triangulation failed");
            }

            Attributes& attributes = result.attributes;

            // WTF is this the only way to get the vertex count?
            std::unordered_set<int> unique_vertices;
            uint32_t num_vertices = 0;
            size_t num_triangles{};
            for (auto& s : result.shapes)
            {
                LOG (DBUG) << "-------Shape " << s.name;

                LOG (DBUG) << "number of tris " << s.mesh.num_face_vertices.size();
                for (auto& id : s.mesh.material_ids)
                {
                    LOG (DBUG) << "material id " << id;
                    Material m = result.materials[id];
                    LOG (DBUG) << m.diffuse[0] << ", " << m.diffuse[1] << ", " <<  m.diffuse[2];

                }

                num_triangles += s.mesh.num_face_vertices.size();
                for (const auto& index : s.mesh.indices)
                {
                    if (unique_vertices.insert (index.position_index).second)
                    {
                        // LOG (DBUG) << index.position_index;
                        ++num_vertices;
                    }
                }
            }

            unique_vertices.clear();

            LOG (DBUG) << num_vertices << " vertices";
            LOG (DBUG) << num_triangles << " triangle";

            std::vector<int> indices;
            indices.reserve (num_triangles * 3);

            MatrixXf V;
            V.resize (3, num_vertices);

            for (auto& s : result.shapes)
            {
                
                for (const auto& index : s.mesh.indices)
                {
                    indices.push_back (index.position_index);
                }

                for (const auto& index : s.mesh.indices)
                {
                    int position_index = index.position_index;

                    if (unique_vertices.insert (position_index).second)
                    {
                        float x = attributes.positions[position_index * 3];
                        float y = attributes.positions[position_index * 3 + 1];
                        float z = attributes.positions[position_index * 3 + 2];

                        V.col (position_index) = Vector3f (x, y, z);
                    }
                }
            }

#if 0
            for (auto& s : result.shapes)
            {
                for (const auto& index : s.mesh.indices)
                {
                    indices.push_back (index.position_index);
                }

                for (const auto& index : s.mesh.indices)
                {
                    int position_index = index.position_index;

                    if (unique_vertices.insert (position_index).second)
                    {
                        float x = attributes.positions[position_index * 3];
                        float y = attributes.positions[position_index * 3 + 1];
                        float z = attributes.positions[position_index * 3 + 2];

                        V.col (position_index) = Vector3f (x, y, z);
                    }
                }
            }


                unique_vertices.clear();

                std::vector<int> indices;
                indices.reserve (s.mesh.indices.size());
                for (const auto& index : s.mesh.indices)
                {
                    indices.push_back (index.position_index);
                }
                uint32_t elementCount = indices.size();
                uint32_t triangleCount = elementCount / 3;
                MatrixXu F;
                F.resize (3, triangleCount);
                std::memcpy (F.data(), indices.data(), elementCount * sizeof (int));

                for (int i = 0; i < F.cols(); ++i)
                {
                    Vector3u tri = F.col (i);
                    
                }

                MatrixXf V;
                V.resize (3, meshVertexCount);
                for (const auto& index : s.mesh.indices)
                {
                    int position_index = index.position_index;
                    if (position_index >= meshVertexCount)
                    {
                        int i = 3;
                    }
                    if (unique_vertices.insert (position_index).second)
                    {
                        float x = attributes.positions[position_index * 3];
                        float y = attributes.positions[position_index * 3 + 1];
                        float z = attributes.positions[position_index * 3 + 2];

                        V.col (position_index) = Vector3f (x, y, z);
                    }
                }

              //  V.resize (0, 0);

              ////  MatrixXf N;  // vertex normals
              ////  MatrixXf FN; // face normals
              ////  generate_normals (F, V, N, FN, false);
              //  //assert (V.cols() == N.cols());

              //  for (int i = 0; i < meshVertexCount; ++i)
              //  {
              //      Vector3f v = V.col (i);
              //    //  Vector3f n = N.col (i);
              //  }
#endif
            //}
        }
        catch (std::exception& e)
        {
            LOG (CRITICAL) << e.what();
        }
    }

    ~Application()
    {
    }

    void onCrash() override
    {
    }

 private:
};

#endif

Jahley::App* Jahley::CreateApplication()
{
    return new Application();
}
