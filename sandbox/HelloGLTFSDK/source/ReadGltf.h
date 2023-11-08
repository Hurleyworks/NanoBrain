#pragma once

#include <sabi_core/sabi_core.h>

using Eigen::MatrixXf;
using Eigen::Vector2f;
using sabi::MeshBuffers;

class ReadGltf
{
 public:
    void read (const std::filesystem::path& filePath);
    void debug();

    const std::vector<MeshBuffers>& getMeshes() const { return meshBuffers; }
    std::vector<MeshBuffers>& getMeshes() { return meshBuffers; }

 private:
    std::vector<MeshBuffers> meshBuffers;
};
