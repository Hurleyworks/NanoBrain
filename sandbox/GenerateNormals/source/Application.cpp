
#include "Jahley.h"
#include <rapidobj/rapidobj.hpp>

// Define application name
const std::string APP_NAME = "GenerateNormals";

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

    pool.wait_for_tasks(); // Wait for tasks to complete
}

// Function to report errors from RapidObj
inline void ReportError (const rapidobj::Error& error)
{
    LOG (CRITICAL) << error.code.message();
    if (!error.line.empty())
    {
        LOG (DBUG) << "On line " << error.line_num << ": \"" << error.line;
    }
}

// Function to count vertices and triangles in the OBJ file
std::pair<uint32_t, uint32_t> getTotalVertexAndTriangleCounts (const rapidobj::Result& result)
{
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

// Function to populate the material ID list
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

// Populate vertex and triangle data
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

// Main application class
class Application : public Jahley::App
{
 public:
    Application() :
        Jahley::App()
    {
        try
        {
            std::string resourceFolder = getResourcePath (APP_NAME);
            LOG (DBUG) << resourceFolder;

            std::filesystem::path filePath = resourceFolder + std::string ("/2surf_cube.obj");
           
            if (!std::filesystem::exists (filePath.generic_string()))
                throw std::runtime_error ("Load failed because file does not exist: " + filePath.generic_string());

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

            std::vector<int32_t> materialIDs (triangleCount);
            getMaterialIdList (result, materialIDs);

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
            generate_normals (F, V, N, FN);

#if 1
            LOG (DBUG) << "--------------------- Face normals";
            for (int i = 0; i < FN.cols(); ++i)
            {
                Vector3f n = FN.col (i);
                LOG (DBUG) << n.x() << ", " << n.y() << ", " << n.z();
            }
            LOG (DBUG) << "--------------------- Vertex normals";
            for (int i = 0; i < N.cols(); ++i)
            {
                Vector3f n = N.col (i);
                LOG (DBUG) << n.x() << ", " << n.y() << ", " << n.z();
            }
#endif

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

// Factory function for Application object
Jahley::App* Jahley::CreateApplication()
{
    return new Application();
}
