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

#include "RenderContext.h"
#include "handlers/Handlers.h"

class Renderer
{
 public:
    Renderer() = default;
    ~Renderer();

    void init (const std::filesystem::path& resourceFolder);
    void setCamera (CameraHandle camera);

    void addRenderableNode (OptiXNode node, const std::filesystem::path& path);
    void addRenderableGeometryInstances (OptiXNode instancedFrom, GeometryInstances& instances);
    void removeRenderableNode (const std::string& name);
    void addSkyDomeImage (const OIIO::ImageBuf&& image);
    void updateMotion();

    void render();

 private:
    RenderContextPtr ctx = nullptr;                           // Rendering ctx
    optixu::HostBlockBuffer2D<shared::PCG32RNG, 1> rngBuffer; // random number generator

    Shared::PipelineLaunchParameters plp; // Pipeline launch parameters
    CUdeviceptr plpOnDevice;              // Device pointer for pipeline launch parameters
    bool restartRender = true;            // Flag to restart render
    uint32_t numAccumFrames = 0;
    float log10EnvLightPowerCoeff = 0.0f;
    float envLightRotation = 0.0f;

    std::unique_ptr<float4[]> renderedPixels = nullptr;

    // Update camera parameters
    void updateCamera (CameraHandle camera)
    {
        camera->getViewMatrix();

        auto& eye = camera->getEyePoint();
        auto& forward = camera->getFoward();
        auto& right = camera->getRight();
        auto& up = camera->getUp();

        Vector3D camRight, camUp, camForward;
        Point3D camEye;

        camUp = Vector3D (up.x(), up.y(), up.z());
        camRight = Vector3D (right.x(), right.y(), right.z());
        camForward = Vector3D (forward.x(), forward.y(), forward.z());
        camEye = Point3D (eye.x(), eye.y(), eye.z());

        plp.camera.aspect = camera->getSensor()->getPixelAspectRatio();
        plp.camera.fovY = camera->getVerticalFOVradians();
        plp.camera.position = camEye;

        // don't understand why but trackball doesn't work correctly
        // unless camRight is negated
        plp.camera.orientation = Matrix3x3 (camRight * -1.0f, camUp, camForward);
        camera->setDirty (false);
        restartRender = true;
    }
};
