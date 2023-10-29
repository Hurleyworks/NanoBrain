#include "GeometryUtilities.h"
#include "OptiXGeometry.h"

using Eigen::Vector2f;

template <typename VertexType, typename TriangleType, typename GeometryData>
void OptiXTriangleMesh<VertexType, TriangleType, GeometryData>::createGeometry (RenderContextPtr ctx, SpaceTime& st, const MaterialInfo& info)
{
    if (!std::filesystem::exists (filePath.generic_string()))
        throw std::runtime_error ("Load failed because file does not exist: " + filePath.generic_string());

    geomInst = ctx->scene.createGeometryInstance();

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
        LOG (CRITICAL) << result.error.code.message();
        throw std::runtime_error ("triangulation failed: " + filePath.generic_string());
    }

    // get vertex and triangle counts and fill in
    // the set of unique vertex indices
    std::unordered_set<int> unique_vertices;
    std::unordered_set<int> unique_uvs;
    uint32_t vertexCount, triangleCount;
    std::tie (vertexCount, triangleCount) = getTotalVertexAndTriangleCounts (result, unique_vertices, unique_uvs);

    std::vector<int32_t> triangleIndices;
    triangleIndices.reserve (triangleCount * 3);
    getTriangleIndices (result, triangleIndices);

    uint32_t elementCount = triangleIndices.size();
    uint32_t tris = elementCount / 3;
    assert (tris == triangleCount);

    MatrixXu F; // faces (triangles in this case)
    F.resize (3, triangleCount);
    std::memcpy (F.data(), triangleIndices.data(), elementCount * sizeof (int));

    MatrixXf V; // vertex positions
    V.resize (3, vertexCount);
    getVertexPositions (result, V, unique_vertices);

    // calc the model bounding box
    st.modelBound.min() = V.rowwise().minCoeff();
    st.modelBound.max() = V.rowwise().maxCoeff();

    MatrixXf N;  // vertex normals
    MatrixXf FN; // face normals
    generate_normals (F, V, N, FN);
    assert (V.cols() == N.cols());

    MatrixXf UV;
    if (unique_uvs.size() == vertexCount)
    {
        UV.resize (2, unique_uvs.size());
        getUVs (result, UV, unique_uvs);
    }
    else
    {
        // else fill with Vector::Zero
        UV.resize (2, vertexCount);
        for (int i = 0; i < vertexCount; ++i)
        {
            UV.col (i) = Vector2f::Zero();
        }
    }

    // normalize and center dyanmic bodies only
    std::string name = filePath.stem().string();
    if (name.find ("static") != 0 && name.find ("STATIC") != 0) // Case-insensitive check for "static" prefix
    {
        // clamp the largest dimension to 1.0f
        float scale = 1.0f;
        normalizeSize (st.modelBound, scale);

        scale *= 0.5f;

        // center vertices on origin
        centerVertices (V, st.modelBound, scale);
    }
    // create OptiX triangles
    std::vector<TriangleType> triangles;
    triangles.reserve (F.cols());
    for (int i = 0; i < F.cols(); ++i)
    {
        Vector3u tri = F.col (i);
        triangles.emplace_back (TriangleType (tri.x(), tri.y(), tri.z()));
    }

    // create OptiX vertices
    std::vector<VertexType> vertices;
    vertices.reserve (V.cols());
    for (int i = 0; i < vertexCount; ++i)
    {
        Vector3f v = V.col (i);
        Vector3f n = N.col (i);
        Vector2f uv = UV.col (i);

        VertexType vertex;
        vertex.position = Point3D(v.x(), v.y(), v.z());
        vertex.normal = Normal3D (n.x(), n.y(), n.z());
        vertex.texCoord = Point2D (uv.y(), uv.x());

        vertices.push_back (vertex);
    }

    // initialize gpu buffers
    triangleBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device, triangles);
    vertexBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device, vertices);

    // generate material list, 1 materialID per triangle
    std::vector<uint8_t> materialIDs (triangleCount);
    getMaterialIdList (result, materialIDs);

    // 1 material ID per triangle
    assert (F.cols() == materialIDs.size());

    // find the number of unique materials
    std::unordered_set<int> uniqueMaterials;
    uint32_t materialCount = 0;
    for (const auto& id : materialIDs)
    {
        if (uniqueMaterials.insert (id).second)
        {
            ++materialCount;
        }
    }

    // looks like a bug in RapidObj that results.materials doesn't get filled in
    if (result.materials.size() == 0)
        materialCount = 1;

    std::vector<optixu::Material> materials;
    if (materialCount > 1)
    {
        materials.reserve (materialCount);
    }

    if (result.materials.size() == 0)
    {
        materialCount = 1;
        materials.push_back (ctx->handlers->mat->createDefaultMaterial<Shared::MaterialData> (info));
    }
    else
    {
        // td::filesystem::path parentDir = filePath.parent_path();
        std::filesystem::path materialFolder (filePath.parent_path());
        for (auto& id : uniqueMaterials)
        {
            if (id >= 0 && id < result.materials.size())
                materials.push_back (ctx->handlers->mat->createMaterial<Shared::MaterialData> (info, result.materials[id], materialFolder));
        }
    }

    GeometryData geomData = {};
    geomData.vertexBuffer = vertexBuffer.getDevicePointer();
    geomData.triangleBuffer = triangleBuffer.getDevicePointer();

    geomInst.setVertexBuffer (vertexBuffer);
    geomInst.setTriangleBuffer (triangleBuffer);

    // only need an matIndexBuffer when there's more than 1 material
    if (materialCount > 1)
    {
        matIndexBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device, materialIDs);
        geomInst.setNumMaterials (materialCount, matIndexBuffer, optixu::IndexSize::k1Byte);
    }
    else
        geomInst.setNumMaterials (1, optixu::BufferView());

    for (int i = 0; i < materials.size(); ++i)
    {
        geomInst.setMaterial (0, i, materials[i]);
    }

    geomInst.setGeometryFlags (0, OPTIX_GEOMETRY_FLAG_NONE);
    geomInst.setUserData (geomData);
}

template <typename VertexType, typename TriangleType, typename GeometryData>
void OptiXTriangleMesh<VertexType, TriangleType, GeometryData>::extractVertexPositions (MatrixXf& V)
{
    uint32_t vertexCount = vertexBuffer.numElements();

    V.resize (3, vertexCount);

    vertexBuffer.map();

    const VertexType* const vertices = vertexBuffer.getMappedPointer();
    for (int i = 0; i < vertexCount; ++i)
    {
        VertexType v = vertices[i];
        V.col (i) = Eigen::Vector3f (v.position.x, v.position.y, v.position.z);
    }

    vertexBuffer.unmap();
}

template <typename VertexType, typename TriangleType, typename GeometryData>
void OptiXTriangleMesh<VertexType, TriangleType, GeometryData>::extractTriangleIndices (MatrixXu& F)
{
    uint32_t triangleCount = triangleBuffer.numElements();

    F.resize (3, triangleCount);

    triangleBuffer.map();

    const TriangleType* const triangles = triangleBuffer.getMappedPointer();
    for (int i = 0; i < triangleCount; ++i)
    {
        TriangleType tri = triangles[i];
        F.col (i) = Vector3u (tri.index0, tri.index1, tri.index2);
    }

    triangleBuffer.unmap();
}

// Explicit Instantiation
template class OptiXTriangleMesh<shared::Vertex, shared::Triangle, Shared::GeometryData>;
