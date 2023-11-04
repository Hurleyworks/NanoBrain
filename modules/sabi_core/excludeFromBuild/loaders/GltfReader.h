/*
MIT License

Copyright (c) 2023 Steve Hurley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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
