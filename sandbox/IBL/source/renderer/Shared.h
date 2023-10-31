#pragma once

// // taken from OptiX_Utility
// https://github.com/shocker-0x15/OptiX_Utility/blob/master/LICENSE.md

#include <common_shared.h>

namespace Shared
{
    static constexpr float probToSampleEnvLight = 0.25f;

    enum RayType
    {
        RayType_Search = 0,
        RayType_Visibility,
        NumRayTypes
    };

    struct PerspectiveCamera
    {
        float aspect;
        float fovY;
        Point3D position;
        Matrix3x3 orientation;

        CUDA_COMMON_FUNCTION Point2D calcScreenPosition (const Point3D& posInWorld) const
        {
            Matrix3x3 invOri = invert (orientation);
            Point3D posInView (invOri * (posInWorld - position));
            Point2D posAtZ1 (posInView.x / posInView.z, posInView.y / posInView.z);
            float h = 2 * std::tan (fovY / 2);
            float w = aspect * h;
            return Point2D (1 - (posAtZ1.x + 0.5f * w) / w,
                            1 - (posAtZ1.y + 0.5f * h) / h);
        }
    };

    struct GeometryData
    {
        const shared::Vertex* vertexBuffer;
        const shared::Triangle* triangleBuffer;
    };

    struct LightSample
    {
        RGB emittance;
        Point3D position;
        Normal3D normal;
        unsigned int atInfinity : 1;
    };

    struct MaterialData
    {
        CUtexObject texture;
        RGB albedo;
        bool isEmitter;

        MaterialData() :
            texture (0),
            albedo (0.0f, 0.0f, 0.5f),
            isEmitter (false) {}
    };

    struct HitPointParams
    {
        RGB albedo;
        Point3D positionInWorld;
        Point3D prevPositionInWorld;
        Normal3D normalInWorld;
        Point2D texCoord;
        uint32_t materialSlot;
    };

    struct PipelineLaunchParameters
    {
        OptixTraversableHandle travHandle;
        int2 imageSize;
        uint32_t numAccumFrames;
        optixu::BlockBuffer2D<shared::PCG32RNG, 1> rngBuffer;
        optixu::NativeBlockBuffer2D<float4> colorAccumBuffer;
        optixu::NativeBlockBuffer2D<float4> albedoAccumBuffer;
        optixu::NativeBlockBuffer2D<float4> normalAccumBuffer;
        PerspectiveCamera camera;
        uint32_t useCameraSpaceNormal : 1;

        // skydome environment
        uint32_t enableEnvLight : 1;
        float envLightPowerCoeff;
        float envLightRotation;
        shared::LightDistribution lightInstDist;
        shared::RegularConstantContinuousDistribution2D envLightImportanceMap;
        CUtexObject envLightTexture;
    };

    struct SearchRayPayload
    {
        RGB alpha;
        RGB contribution;
        Point3D origin;
        Vector3D direction;
        struct
        {
            uint32_t pathLength : 30;
            uint32_t terminate : 1;
        };
    };

    using SearchRayPayloadSignature = optixu::PayloadSignature<shared::PCG32RNG, SearchRayPayload*, HitPointParams*, RGB*, Normal3D*>;
    using VisibilityRayPayloadSignature = optixu::PayloadSignature<float>;

} // namespace Shared
