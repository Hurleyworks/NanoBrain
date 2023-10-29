#pragma once

// taken from OptiX_Utility
// https://github.com/shocker-0x15/OptiX_Utility/blob/master/LICENSE.md
// and from Shocker GfxExp
// https://github.com/shocker-0x15/GfxExp

#include "../Shared.h"
#include <common_device.h>

using namespace Shared;
using namespace shared;

RT_PIPELINE_LAUNCH_PARAMETERS PipelineLaunchParameters plp;

static constexpr bool useSolidAngleSampling = false;
static constexpr bool useImplicitLightSampling = true;
static constexpr bool useExplicitLightSampling = true;
static constexpr bool useMultipleImportanceSampling = useImplicitLightSampling && useExplicitLightSampling;
static_assert (useImplicitLightSampling || useExplicitLightSampling, "Invalid configuration for light sampling.");

class LambertBRDF
{
    RGB m_reflectance;

 public:
    CUDA_DEVICE_FUNCTION LambertBRDF (const RGB& reflectance) :
        m_reflectance (reflectance) {}

    CUDA_DEVICE_FUNCTION void getSurfaceParameters (RGB* diffuseReflectance, RGB* specularReflectance, float* roughness) const
    {
        *diffuseReflectance = m_reflectance;
        *specularReflectance = RGB (0.0f, 0.0f, 0.0f);
        *roughness = 1.0f;
    }

    CUDA_DEVICE_FUNCTION RGB sampleThroughput (
        const Vector3D& vGiven, float uDir0, float uDir1,
        Vector3D* vSampled, float* dirPDensity) const
    {
        *vSampled = cosineSampleHemisphere (uDir0, uDir1);
        *dirPDensity = vSampled->z / Pi;
        if (vGiven.z <= 0.0f)
            vSampled->z *= -1;
        return m_reflectance;
    }

    CUDA_DEVICE_FUNCTION RGB evaluate (const Vector3D& vGiven, const Vector3D& vSampled) const
    {
        if (vGiven.z * vSampled.z > 0)
            return m_reflectance / Pi;
        else
            return RGB (0.0f, 0.0f, 0.0f);
    }
    CUDA_DEVICE_FUNCTION float evaluatePDF (const Vector3D& vGiven, const Vector3D& vSampled) const
    {
        if (vGiven.z * vSampled.z > 0)
            return fabs (vSampled.z) / Pi;
        else
            return 0.0f;
    }

    CUDA_DEVICE_FUNCTION RGB evaluateDHReflectanceEstimate (const Vector3D& vGiven) const
    {
        return m_reflectance;
    }
};


template <bool computeHypotheticalAreaPDensity, bool useSolidAngleSampling>
CUDA_DEVICE_FUNCTION CUDA_INLINE void computeSurfacePoint (
    const Shared::GeometryData& geomInst,
    uint32_t primIndex, float b1, float b2,
    const Point3D& referencePoint,
    Point3D* positionInWorld, Normal3D* shadingNormalInWorld, Vector3D* texCoord0DirInWorld,
    Normal3D* geometricNormalInWorld, Point2D* texCoord,
    float* hypAreaPDensity)
{
    const Triangle& tri = geomInst.triangleBuffer[primIndex];
    const Vertex& v0 = geomInst.vertexBuffer[tri.index0];
    const Vertex& v1 = geomInst.vertexBuffer[tri.index1];
    const Vertex& v2 = geomInst.vertexBuffer[tri.index2];
    const Point3D p[3] = {
        transformPointFromObjectToWorldSpace (v0.position),
        transformPointFromObjectToWorldSpace (v1.position),
        transformPointFromObjectToWorldSpace (v2.position),
    };
    float b0 = 1 - (b1 + b2);

    // EN: Compute hit point properties in the local coordinates.
    *positionInWorld = b0 * p[0] + b1 * p[1] + b2 * p[2];
    Normal3D shadingNormal = b0 * v0.normal + b1 * v1.normal + b2 * v2.normal;
    Vector3D texCoord0Dir = b0 * v0.texCoord0Dir + b1 * v1.texCoord0Dir + b2 * v2.texCoord0Dir;
    Normal3D geometricNormal (cross (p[1] - p[0], p[2] - p[0]));
    float area;
    if constexpr (computeHypotheticalAreaPDensity && !useSolidAngleSampling)
        area = 0.5f * length (geometricNormal);
    else
        (void)area;
    *texCoord = b0 * v0.texCoord + b1 * v1.texCoord + b2 * v2.texCoord;

    // EN: Convert the local properties to ones in world coordinates.
    *shadingNormalInWorld = normalize (transformNormalFromObjectToWorldSpace (shadingNormal));
    *texCoord0DirInWorld = normalize (transformVectorFromObjectToWorldSpace (texCoord0Dir));
    *geometricNormalInWorld = normalize (geometricNormal);
    if (!shadingNormalInWorld->allFinite())
    {
        *shadingNormalInWorld = Normal3D (0, 0, 1);
        *texCoord0DirInWorld = Vector3D (1, 0, 0);
    }
    if (!texCoord0DirInWorld->allFinite())
    {
        Vector3D bitangent;
        makeCoordinateSystem (*shadingNormalInWorld, texCoord0DirInWorld, &bitangent);
    }

    if constexpr (computeHypotheticalAreaPDensity)
    {
        // EN: Compute a hypothetical probability density with which the intersection point
        //     is sampled by explicit light sampling.
        float lightProb = 1.0f;
        if (plp.envLightTexture && plp.enableEnvLight)
            lightProb *= (1 - probToSampleEnvLight);

        if (!isfinite (lightProb))
        {
            *hypAreaPDensity = 0.0f;
            return;
        }

        *hypAreaPDensity = lightProb / area;
    }
}

// Define a struct called HitPointParameter to hold hit point info
struct HitPointParameter
{
    float b1, b2;      // Barycentric coordinates
    int32_t primIndex; // Index of the primitive hit by the ray

    // Static member function to get hit point parameters
    CUDA_DEVICE_FUNCTION CUDA_INLINE static HitPointParameter get()
    {
        HitPointParameter ret; // Create an instance of the struct

        // Get barycentric coordinates from OptiX API
        float2 bc = optixGetTriangleBarycentrics();

        // Store the barycentric coordinates in the struct
        ret.b1 = bc.x;
        ret.b2 = bc.y;

        // Get the index of the primitive hit by the ray from OptiX API
        ret.primIndex = optixGetPrimitiveIndex();

        // Return the populated struct
        return ret;
    }
};
;

// This struct is used to fetch geometry and material data from
// the Shader Binding Table (SBT) in OptiX.
struct HitGroupSBTRecordData
{
    GeometryData geomData;        // Geometry data for the hit object
    Shared::MaterialData matData; // Material data for the hit object

    // Static member function to retrieve the SBT record data
    CUDA_DEVICE_FUNCTION CUDA_INLINE static const HitGroupSBTRecordData& get()
    {
        // Use optixGetSbtDataPointer() to get the pointer to the SBT data
        // Cast the pointer to type HitGroupSBTRecordData and dereference it
        return *reinterpret_cast<HitGroupSBTRecordData*> (optixGetSbtDataPointer());
    }
};

// Define the main CUDA device kernel for path tracing
CUDA_DEVICE_KERNEL void RT_RG_NAME (pathTracing)()
{
    // Get the launch index for this thread
    uint2 launchIndex = make_uint2 (optixGetLaunchIndex().x, optixGetLaunchIndex().y);

    // Initialize the random number generator
    PCG32RNG rng = plp.rngBuffer[launchIndex];

    // Get camera properties from the pipeline parameters
    const PerspectiveCamera& camera = plp.camera;

    // Generate random numbers for jittering
    float jx = rng.getFloat0cTo1o();
    float jy = rng.getFloat0cTo1o();

    // Update the RNG buffer
    plp.rngBuffer.write (launchIndex, rng);

    // Compute normalized screen coordinates
    float x = (launchIndex.x + jx) / plp.imageSize.x;
    float y = (launchIndex.y + jy) / plp.imageSize.y;

    // Compute vertical and horizontal view angles
    float vh = 2 * std::tan (plp.camera.fovY * 0.5f);
    float vw = plp.camera.aspect * vh;

    // Setup ray origin and direction
    Point3D origin = camera.position;
    Vector3D direction = normalize (camera.orientation * Vector3D (vw * (0.5f - x), vh * (0.5f - y), 1));

    // Initialize ray payload
    SearchRayPayload payload;
    payload.alpha = RGB (1.0f, 1.0f, 1.0f);
    payload.contribution = RGB (0.0f, 0.0f, 0.0f);
    payload.pathLength = 1;
    payload.terminate = false;
    SearchRayPayload* payloadPtr = &payload;

    RGB firstHitAlbedo (0.0f, 0.0f, 0.0f);
    Normal3D firstHitNormal (0.0f, 0.0f, 0.0f);
    RGB* firstHitAlbedoPtr = &firstHitAlbedo;
    Normal3D* firstHitNormalPtr = &firstHitNormal;

    // Initialize variables for storing hit point properties
    HitPointParams hitPointParams;
    hitPointParams.positionInWorld = Point3D (NAN);
    hitPointParams.prevPositionInWorld = Point3D (NAN);
    hitPointParams.normalInWorld = Normal3D (NAN);
    hitPointParams.texCoord = Point2D (NAN);
    hitPointParams.materialSlot = 0xFFFFFFFF;
    HitPointParams* hitPointParamsPtr = &hitPointParams;

    // Main path tracing loop
    while (true)
    {
        // Trace the ray and collect results
        SearchRayPayloadSignature::trace (
            plp.travHandle, origin.toNative(), direction.toNative(),
            0.0f, FLT_MAX, 0.0f, 0xFF, OPTIX_RAY_FLAG_NONE,
            RayType_Search, NumRayTypes, RayType_Search,
            rng, payloadPtr, hitPointParamsPtr, firstHitAlbedoPtr, firstHitNormalPtr);

        // Break out of the loop if conditions are met
        if (payload.terminate || payload.pathLength >= 10)
            break;

        // Update ray origin and direction for the next iteration
        origin = payload.origin;
        direction = payload.direction;
        ++payload.pathLength;
    }

    // Store the updated RNG state back to the buffer
    plp.rngBuffer[launchIndex] = rng;

    // Output information for the denoiser
    firstHitNormal = transpose (camera.orientation) * hitPointParams.normalInWorld;
    firstHitNormal.x *= -1;

    RGB prevAlbedoResult (0.0f, 0.0f, 0.0f);
    RGB prevColorResult (0.0f, 0.0f, 0.0f);
    Normal3D prevNormalResult (0.0f, 0.0f, 0.0f);

    if (plp.numAccumFrames > 0)
    {
        prevColorResult = RGB (getXYZ (plp.colorAccumBuffer.read (launchIndex)));
        prevAlbedoResult = RGB (getXYZ (plp.albedoAccumBuffer.read (launchIndex)));
        prevNormalResult = Normal3D (getXYZ (plp.normalAccumBuffer.read (launchIndex)));
    }

    float curWeight = 1.0f / (1 + plp.numAccumFrames);
    RGB colorResult = (1 - curWeight) * prevColorResult + curWeight * payload.contribution;
    RGB albedoResult = (1 - curWeight) * prevAlbedoResult + curWeight * firstHitAlbedo;
    Normal3D normalResult = (1 - curWeight) * prevNormalResult + curWeight * firstHitNormal;

    plp.colorAccumBuffer.write (launchIndex, make_float4 (colorResult.toNative(), 1.0f));
    plp.albedoAccumBuffer.write (launchIndex, make_float4 (albedoResult.toNative(), 1.0f));
    plp.normalAccumBuffer.write (launchIndex, make_float4 (normalResult.toNative(), 1.0f));
}

// CUDA kernel for a miss progrm in OptiX
CUDA_DEVICE_KERNEL void RT_MS_NAME (miss)()
{
    // Declare pointers for the ray payload and hit point parameters
    SearchRayPayload* payload;
    HitPointParams* hitPntParams;

    // Retrieve ray payload and hit point parameters
    SearchRayPayloadSignature::get (nullptr, &payload, &hitPntParams, nullptr, nullptr);

    // If there's no environment light texture, apply some basic ambient light and terminate
    if (plp.envLightTexture == 0)
    {
        payload->contribution += payload->alpha * RGB (0.01f, 0.015f, 0.02f);
        payload->terminate = true;
        return;
    }
    // Store the normalized direction as the surface normal
    // Get the inverse of the ray direction in world space
    Vector3D vOut (-Vector3D (optixGetWorldRayDirection()));
    hitPntParams->normalInWorld = Normal3D (vOut);

    Vector3D rayDir = normalize (Vector3D (optixGetWorldRayDirection()));
    float posPhi, theta;
    toPolarYUp (rayDir, &posPhi, &theta);

    float phi = posPhi + plp.envLightRotation;
    phi = phi - floorf (phi / (2 * Pi)) * 2 * Pi;
    Point2D texCoord (phi / (2 * Pi), theta / Pi);

    
    float4 texValue = tex2DLod<float4> (plp.envLightTexture, texCoord.x, texCoord.y, 0.0f);
    RGB luminance = plp.envLightPowerCoeff * RGB (texValue.x, texValue.y, texValue.z);
    float misWeight = 1.0f;
    if constexpr (true)
    {
        float uvPDF = plp.envLightImportanceMap.evaluatePDF (texCoord.x, texCoord.y);
        float hypAreaPDensity = uvPDF / (2 * Pi * Pi * std::sin (theta));
        // FIXME

        float lightPDensity =
            (plp.lightInstDist.integral() > 0.0f ? probToSampleEnvLight : 1.0f) *
            hypAreaPDensity;
        // FIXME
        // float bsdfPDensity = rwPayload->prevDirPDensity;
        float bsdfPDensity = 0.5f; // just guessing
        misWeight = pow2 (bsdfPDensity) / (pow2 (bsdfPDensity) + pow2 (lightPDensity));
    }
    payload->contribution += payload->alpha * luminance * misWeight;

    // Terminate the ray
    payload->terminate = true;
}

// CUDA kernel for a closest hit progrm in OptiX
CUDA_DEVICE_KERNEL void RT_CH_NAME (shading)()
{
    auto sbtr = HitGroupSBTRecordData::get();
    const Shared::MaterialData& mat = sbtr.matData;
    const GeometryData& geom = sbtr.geomData;

    PCG32RNG rng;
    SearchRayPayload* payload;
    RGB* firstHitAlbedo;
    Normal3D* firstHitNormal;
    HitPointParams* hitPntParams;
    SearchRayPayloadSignature::get (&rng, &payload, &hitPntParams, &firstHitAlbedo, &firstHitNormal);

    const Point3D rayOrigin (optixGetWorldRayOrigin());

    auto hp = HitPointParameter::get();
    Point3D positionInWorld;
    Normal3D shadingNormalInWorld;
    Vector3D texCoord0DirInWorld;
    Normal3D geometricNormalInWorld;
    Point2D texCoord;
    float hypAreaPDensity;
    computeSurfacePoint<useMultipleImportanceSampling, useSolidAngleSampling> (
        geom, hp.primIndex, hp.b1, hp.b2,
        rayOrigin,
        &positionInWorld, &shadingNormalInWorld, &texCoord0DirInWorld,
        &geometricNormalInWorld, &texCoord, &hypAreaPDensity);
    if constexpr (!useMultipleImportanceSampling)
        (void)hypAreaPDensity;

    Vector3D vOut = normalize (-Vector3D (optixGetWorldRayDirection()));
    float frontHit = dot (vOut, geometricNormalInWorld) >= 0.0f ? 1.0f : -1.0f;

    ReferenceFrame shadingFrame (shadingNormalInWorld, texCoord0DirInWorld);

    positionInWorld = offsetRayOrigin (positionInWorld, frontHit * geometricNormalInWorld);
    Vector3D vOutLocal = shadingFrame.toLocal (vOut);

    RGB albedo;
    if (mat.texture)
        albedo = RGB (getXYZ (tex2DLod<float4> (mat.texture, texCoord.x, texCoord.y, 0.0f)));
    else
        albedo = RGB (mat.albedo);

    // generate next ray.
    Vector3D vInLocal;
    float dirPDensity;
    LambertBRDF bsdf (albedo);
    payload->alpha *= bsdf.sampleThroughput (
        vOutLocal, rng.getFloat0cTo1o(), rng.getFloat0cTo1o(),
        &vInLocal, &dirPDensity);
    Vector3D vIn = shadingFrame.fromLocal (vInLocal);

    // A simplified example of how you might sample the environment map
    // based on the surface normal to simulate Lambertian reflection:
    float posPhi, posTheta;
    toPolarYUp (Vector3D (shadingNormalInWorld), &posPhi, &posTheta);

    float ph = posPhi + plp.envLightRotation;

    float u = ph / (2 * Pi);
    u -= floorf (u);
    float v = posTheta / Pi;

    float4 texValue = tex2DLod<float4> (plp.envLightTexture, u, v, 0.0f);
    RGB environmentLight (texValue.x, texValue.y, texValue.z);
    environmentLight *= plp.envLightPowerCoeff;

    RGB lambertReflection (environmentLight / Pi);

    // Update payload's contribution using Lambert's reflection
    payload->contribution += payload->alpha * albedo * lambertReflection;

    hitPntParams->normalInWorld = shadingNormalInWorld;
    if (payload->pathLength == 1)
    {
        *firstHitAlbedo = albedo;
        *firstHitNormal = shadingNormalInWorld;
    }

    payload->origin = positionInWorld;
    payload->direction = vIn;
    payload->terminate = false;

    SearchRayPayloadSignature::set (&rng, nullptr, nullptr, nullptr, nullptr);
}

CUDA_DEVICE_KERNEL void RT_AH_NAME (visibility)()
{
    float visibility = 0.0f;
    VisibilityRayPayloadSignature::set (&visibility);

    optixTerminateRay();
}
