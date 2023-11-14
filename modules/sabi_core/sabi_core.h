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

#include "../wabi_core/wabi_core.h"
#include <rapidobj/rapidobj.hpp>
#include <cgltfReader/cgltf.h>

// cereal
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

constexpr float DEFAULT_ZOOM_FACTOR = 0.5f;
constexpr float DEFAULT_ZOOM_MULTIPLIER = 200.0f;

const Eigen::Vector3f DEFAULT_CAMERA_POSIIION = Eigen::Vector3f (2.0f, 3.0f, -4.0f);
const Eigen::Vector3f DEFAULT_CAMERA_TARGET = Eigen::Vector3f::Zero();
constexpr float DEFAULT_FOV_DEGREES = 45.0f;
constexpr float DEFAULT_ASPECT = (float)DEFAULT_DESKTOP_WINDOW_WIDTH / (float)DEFAULT_DESKTOP_WINDOW_HEIGHT;
constexpr float DEFAULT_NEAR_PLANE = 0.01f;
constexpr float DEFAULT_FAR_PLANE = 1000.0f;
constexpr float DEFAULT_FOCAL_LENGTH = 1.0f;
constexpr float DEFAULT_APETURE = 0.0f;

// must specialize for Eigen
namespace cereal
{
    template <class Archive>
    void serialize (Archive& ar, Eigen::Vector3f& vector)
    {
        ar (cereal::make_nvp ("x", vector.x()),
            cereal::make_nvp ("y", vector.y()),
            cereal::make_nvp ("z", vector.z()));
    }
}


namespace sabi
{
    // a Surface is a group of triangles with
    // a unique Material
    struct Surface
    {
        MatrixXu F; // triangle indices
        cgltf_material material;
        std::vector<Eigen::Vector2f> uvs; // UV coordinates
    };

    struct MeshBuffers
    {
        MatrixXf V;  // vertices
        MatrixXf N;  // vertex normals
        MatrixXf FN; // face  normals
        std::vector<Surface> surfaces;
        Eigen::Affine3f transform;
    };

// camera
#include "excludeFromBuild/camera/CameraSensor.h"
#include "excludeFromBuild/camera/CameraBody.h"
#include "excludeFromBuild/loaders/GltfReader.h"

} // namespace sabi
