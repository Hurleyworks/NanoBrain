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
    optixu::HostBlockBuffer2D<Shared::PCG32RNG, 1> rngBuffer; // random number generator

    Shared::PipelineLaunchParameters plp; // Pipeline launch parameters
    CUdeviceptr plpOnDevice;              // Device pointer for pipeline launch parameters
    bool restartRender = true;            // Flag to restart render
    uint32_t numAccumFrames = 0;
    float log10EnvLightPowerCoeff = 0.55f;
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

        float3 camRight, camUp, camForward, camEye;

        camUp = make_float3 (up.x(), up.y(), up.z());
        camRight = make_float3 (right.x(), right.y(), right.z());
        camForward = make_float3 (forward.x(), forward.y(), forward.z());
        camEye = make_float3 (eye.x(), eye.y(), eye.z());

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
