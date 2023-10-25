#pragma once

// taken from OptiX_Utility
// https://github.com/shocker-0x15/OptiX_Utility/blob/master/LICENSE.md

#include "../Shared.h"

using namespace Shared;

RT_PIPELINE_LAUNCH_PARAMETERS PipelineLaunchParameters plp;

struct HitPointParameter
{
    float b1, b2;
    int32_t primIndex;

    CUDA_DEVICE_FUNCTION CUDA_INLINE static HitPointParameter get()
    {
        HitPointParameter ret;
        float2 bc = optixGetTriangleBarycentrics();
        ret.b1 = bc.x;
        ret.b2 = bc.y;
        ret.primIndex = optixGetPrimitiveIndex();
        return ret;
    }
};

struct HitGroupSBTRecordData
{
    GeometryData geomData;
    MaterialData matData;

    CUDA_DEVICE_FUNCTION CUDA_INLINE static const HitGroupSBTRecordData& get()
    {
        return *reinterpret_cast<HitGroupSBTRecordData*> (optixGetSbtDataPointer());
    }
};

CUDA_DEVICE_KERNEL void RT_RG_NAME (pathTracing)()
{
    uint2 launchIndex = make_uint2 (optixGetLaunchIndex().x, optixGetLaunchIndex().y);

    PCG32RNG rng = plp.rngBuffer[launchIndex];

    float jx = rng.getFloat0cTo1o();
    float jy = rng.getFloat0cTo1o();
    plp.rngBuffer.write (launchIndex, rng);

    float x = (launchIndex.x + jx) / plp.imageSize.x;
    float y = (launchIndex.y + jy) / plp.imageSize.y;
    float vh = 2 * std::tan (plp.camera.fovY * 0.5f);
    float vw = plp.camera.aspect * vh;

    float3 origin = plp.camera.position;
    float3 direction = normalize (plp.camera.orientation * make_float3 (vw * (0.5f - x), vh * (0.5f - y), 1));

    SearchRayPayload payload;
    payload.alpha = make_float3 (1.0f, 1.0f, 1.0f);
    payload.contribution = make_float3 (0.0f, 0.0f, 0.0f);
    payload.pathLength = 1;
    payload.terminate = false;
    SearchRayPayload* payloadPtr = &payload;
    float3 firstHitAlbedo = make_float3 (0.0f, 0.0f, 0.0f);
    float3 firstHitNormal = make_float3 (0.0f, 0.0f, 0.0f);
    float3* firstHitAlbedoPtr = &firstHitAlbedo;
    float3* firstHitNormalPtr = &firstHitNormal;
    while (true)
    {
        SearchRayPayloadSignature::trace (
            plp.travHandle, origin, direction,
            0.0f, FLT_MAX, 0.0f, 0xFF, OPTIX_RAY_FLAG_NONE,
            RayType_Search, NumRayTypes, RayType_Search,
            rng, payloadPtr, firstHitAlbedoPtr, firstHitNormalPtr);
        if (payload.terminate || payload.pathLength >= 10)
            break;

        origin = payload.origin;
        direction = payload.direction;
        ++payload.pathLength;
    }

    plp.rngBuffer[launchIndex] = rng;

    if (plp.useCameraSpaceNormal)
    {
        // Convert the normal into the camera space (right handed, looking down the negative Z-axis).
        firstHitNormal = transpose (plp.camera.orientation) * firstHitNormal;
        firstHitNormal.x *= -1;
    }

    float3 prevColorResult = make_float3 (0.0f, 0.0f, 0.0f);
    float3 prevAlbedoResult = make_float3 (0.0f, 0.0f, 0.0f);
    float3 prevNormalResult = make_float3 (0.0f, 0.0f, 0.0f);
    if (plp.numAccumFrames > 0)
    {
        prevColorResult = getXYZ (plp.colorAccumBuffer.read (launchIndex));
        prevAlbedoResult = getXYZ (plp.albedoAccumBuffer.read (launchIndex));
        prevNormalResult = getXYZ (plp.normalAccumBuffer.read (launchIndex));
    }
    float curWeight = 1.0f / (1 + plp.numAccumFrames);
    float3 colorResult = (1 - curWeight) * prevColorResult + curWeight * payload.contribution;
    float3 albedoResult = (1 - curWeight) * prevAlbedoResult + curWeight * firstHitAlbedo;
    float3 normalResult = (1 - curWeight) * prevNormalResult + curWeight * firstHitNormal;
    plp.colorAccumBuffer.write (launchIndex, make_float4 (colorResult, 1.0f));
    plp.albedoAccumBuffer.write (launchIndex, make_float4 (albedoResult, 1.0f));
    plp.normalAccumBuffer.write (launchIndex, make_float4 (normalResult, 1.0f));
}

CUDA_DEVICE_FUNCTION void toPolarYUp (const float3& v, float* phi, float* theta)
{
    *theta = std::acos (min (max (v.y, -1.0f), 1.0f));
    *phi = std::fmod (std::atan2 (-v.x, v.z) + 2 * Pi,
                      2 * Pi);
}

CUDA_DEVICE_KERNEL void RT_MS_NAME (miss)()
{
    SearchRayPayload* payload;
    float3* albedo;
    float3* normal;
    SearchRayPayloadSignature::get (nullptr, &payload, &albedo, &normal);

    if (plp.envLightTexture == 0)
    {
        payload->contribution += payload->alpha * make_float3 (0.01f, 0.015f, 0.02f);
        payload->terminate = true;
        return;
    }

    float3 p = optixGetWorldRayDirection();

    float posPhi, posTheta;
    toPolarYUp (p, &posPhi, &posTheta);

    float phi = posPhi + plp.envLightRotation;

    float u = phi / (2 * Pi);
    u -= floorf (u);
    float v = posTheta / Pi;

    //  if (plp.numAccumFrames == 2)
    //     pixelprintf (launchIndex, 100, 100, "%f-%f-%u \n", u, v, plp.numAccumFrames);

    float4 texValue = tex2DLod<float4> (plp.envLightTexture, u, v, 0.0f);
    float3 luminance = make_float3 (texValue);
    luminance *= plp.envLightPowerCoeff;

    payload->contribution += payload->alpha * luminance / Pi;

    payload->terminate = true;
}

CUDA_DEVICE_KERNEL void RT_CH_NAME (shading)()
{
    auto sbtr = HitGroupSBTRecordData::get();
    const MaterialData& mat = sbtr.matData;
    const GeometryData& geom = sbtr.geomData;

    PCG32RNG rng;
    SearchRayPayload* payload;
    float3* firstHitAlbedo;
    float3* firstHitNormal;
    SearchRayPayloadSignature::get (&rng, &payload, &firstHitAlbedo, &firstHitNormal);

    auto hp = HitPointParameter::get();
    float3 hitPointWorld;
    float3 surfaceNormalWorld;
    float2 texCoord;
    {
        const Triangle& tri = geom.triangleBuffer[hp.primIndex];
        const Vertex& v0 = geom.vertexBuffer[tri.index0];
        const Vertex& v1 = geom.vertexBuffer[tri.index1];
        const Vertex& v2 = geom.vertexBuffer[tri.index2];
        float b1 = hp.b1;
        float b2 = hp.b2;
        float b0 = 1 - (b1 + b2);
        hitPointWorld = b0 * v0.position + b1 * v1.position + b2 * v2.position;
        surfaceNormalWorld = b0 * v0.normal + b1 * v1.normal + b2 * v2.normal;
        texCoord = b0 * v0.texCoord + b1 * v1.texCoord + b2 * v2.texCoord;

        hitPointWorld = optixTransformPointFromObjectToWorldSpace (hitPointWorld);
        surfaceNormalWorld = normalize (optixTransformNormalFromObjectToWorldSpace (surfaceNormalWorld));
    }

    // From ChatGPT4
    // Basically, this chunk of code helps determine if the ray hits the front or back
    // of a surface and adjusts the normal and hit point accordingly.

    // Ah, got it. If you used the incoming ray direction optixGetWorldRayDirection()
    // instead of its negative -optixGetWorldRayDirection() for isFrontFace, the logic would flip.
    // Specifically, a positive dot product would then indicate a back-face hit, and a negative
    // or zero would indicate a front-face hit. This is because the incoming ray direction and
    // the surface normal would be pointing in roughly the same direction for back-face hits,
    // making the dot product positive.

    // So yes, you could use the incoming ray direction, but you'd need to
    // adjust the logic of the isFrontFace check accordingly.

    // vOut is the opposite of the incoming ray direction in world coordinates.
    float3 vOut = -optixGetWorldRayDirection();

    // This line checks if the ray hits the front face of the surface.
    // If the dot product is positive, it means the surface normal and
    // the ray direction are somewhat aligned, indicating a front-face hit.
    bool isFrontFace = dot (vOut, surfaceNormalWorld) > 0;

    // If it's not a front-face hit, the code flips the normal to point outward.
    if (!isFrontFace)
        surfaceNormalWorld = -surfaceNormalWorld;

    // This nudges the hit point slightly along the surface normal to
    // avoid self-intersection in future ray casts.
    hitPointWorld = hitPointWorld + surfaceNormalWorld * 0.001f;

    float3 albedo;
    if (mat.texture)
        albedo = getXYZ (tex2DLod<float4> (mat.texture, texCoord.x, texCoord.y, 0.0f));
    else
        albedo = mat.albedo;

    if (payload->pathLength == 1)
    {
        *firstHitAlbedo = albedo;
        *firstHitNormal = surfaceNormalWorld;
    }
   
    // From ChatGPT4
    // Here's a simplified example of how you might sample the environment map
    // based on the surface normal to simulate Lambertian reflection:
    float posPhi, posTheta;
    toPolarYUp (surfaceNormalWorld, &posPhi, &posTheta);

    float ph = posPhi + plp.envLightRotation;

    float u = ph / (2 * Pi);
    u -= floorf (u);
    float v = posTheta / Pi;

    float4 texValue = tex2DLod<float4> (plp.envLightTexture, u, v, 0.0f);
    float3 environmentLight = make_float3 (texValue);
    environmentLight *= plp.envLightPowerCoeff;

    float3 lambertReflection = environmentLight / Pi;

    // Update payload's contribution using Lambert's reflection
    payload->contribution += payload->alpha * albedo * lambertReflection;

    // This lambda function generates a local coordinate system (s, t, n) based on the given normal n.
    // This is useful for transforming vectors from one coordinate system to another, like from
    // the global coordinate system to a surface-local one.
    const auto makeCoordinateSystem = [] (const float3& n, float3* s, float3* t)
    {
        // Here, sign, a, and b are calculated to construct the s and t vectors.
        // They are part of the mathematical magic that simplifies the coordinate system generation..
        float sign = n.z >= 0 ? 1 : -1;
        float a = -1 / (sign + n.z);
        float b = n.x * n.y * a;

        // s and t are the local coordinate system basis vectors perpendicular to n.
        *s = make_float3 (1 + sign * n.x * n.x * a, sign * b, -sign * n.x);
        *t = make_float3 (b, sign + n.y * n.y * a, -n.y);
    };

    float3 s;
    float3 t;
    makeCoordinateSystem (surfaceNormalWorld, &s, &t);

    // generate random incoming direction
    // phi and theta are random angles, generated to produce a random incoming direction vIn.
    float phi = 2 * Pi * rng.getFloat0cTo1o();
    float theta = std::asin (std::sqrt (rng.getFloat0cTo1o()));
    float sinTheta = std::sin (theta);

    // Here, vIn is calculated in spherical coordinates. This is a general way to express directions in 3D space.
    float3 vIn = make_float3 (std::cos (phi) * sinTheta, std::sin (phi) * sinTheta, std::cos (theta));

    // the ultimate goal is to get vIn in world coordinates, but this code is doing it in a roundabout way.
    // It initially calculates vIn in a local coordinate system where the surface normal (surfaceNormalWorld) is the z-axis.
    // This makes the math for random sampling easier.

    // This code is actually converting vIn back to world coordinates.
    // It does this by taking the vIn defined in this local coordinate system
    // and dotting it with each of the basis vectors (s, t, surfaceNormalWorld) to get the
    // components in the world coordinates.

    // So, to sum it up, vIn starts out in a convenient local coordinate system for the calculations
    // and then gets transformed back to world coordinates.
    vIn = make_float3 (dot (make_float3 (s.x, t.x, surfaceNormalWorld.x), vIn),
                       dot (make_float3 (s.y, t.y, surfaceNormalWorld.y), vIn),
                       dot (make_float3 (s.z, t.z, surfaceNormalWorld.z), vIn));

    payload->alpha = payload->alpha * albedo;
    payload->origin = hitPointWorld;
    payload->direction = vIn;
    payload->terminate = false;

    SearchRayPayloadSignature::set (&rng, nullptr, nullptr, nullptr);
}

CUDA_DEVICE_KERNEL void RT_AH_NAME (visibility)()
{
    float visibility = 0.0f;
    VisibilityRayPayloadSignature::set (&visibility);

    optixTerminateRay();
}
