#pragma once

#include <mace_core/mace_core.h>
#include "cgltf.h"

using Eigen::MatrixXf;
using Eigen::Vector2f;
using MatrixXu = Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3u = Eigen::Matrix<uint32_t, 3, 1>;

class ReadGltf
{
 public:
    struct Surface
    {
        MatrixXu F; // triangle indices
        cgltf_material material;
        std::vector<Vector2f> uvs; // UV coordinates
    };

    struct MeshBuffers
    {
        MatrixXf V; // vertices
        MatrixXf N; // vertex normals

        std::vector<Surface> surfaces;
    };

 public:
    void read (const std::filesystem::path& filePath);
    void debug();

 private:
    std::vector<MeshBuffers> meshBuffers;

    void getVertexAttributes (Eigen::MatrixXf& matrix, const cgltf_accessor* accessor);
    void getTriangleIndices (MatrixXu& matrix, cgltf_accessor* accessor);
    void getUVs (std::vector<Vector2f>& vec, cgltf_accessor* accessor);
    void debugMaterial (const cgltf_material* material);
};
