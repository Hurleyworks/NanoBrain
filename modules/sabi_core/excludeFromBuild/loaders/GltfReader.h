#pragma once

#include "../../sabi_core.h"

using Eigen::MatrixXf;
using Eigen::Vector2f;
using sabi::MeshBuffers;

class GltfReader
{
 public:
    void read (const std::filesystem::path& filePath);
    void debug();
    const std::vector<MeshBuffers>& getMeshes() const { return meshBuffers; }
    std::vector<MeshBuffers>& getMeshes() { return meshBuffers; }

 private:
    std::vector<MeshBuffers> meshBuffers;

    void getVertexAttributes (Eigen::MatrixXf& matrix, const cgltf_accessor* accessor);
    void getTriangleIndices (MatrixXu& matrix, cgltf_accessor* accessor);
    void getUVs (std::vector<Vector2f>& vec, cgltf_accessor* accessor);
    void debugMaterial (const cgltf_material* material);
};
