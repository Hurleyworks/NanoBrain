
// taken from OptiX_Utility
// https://github.com/shocker-0x15/OptiX_Utility/blob/master/LICENSE.md
// and from Shocker GfxExp
// https://github.com/shocker-0x15/GfxExp

#include "../Shared.h"
#include <common_device.h>

using namespace Shared;
using namespace shared;

RT_PIPELINE_LAUNCH_PARAMETERS PipelineLaunchParameters plp;

class LambertBRDF
{
    RGB m_reflectance; // The diffuse reflectance of the surface

 public:
    // Constructor: Initializes the Lambertian BRDF with a given reflectance value
    CUDA_DEVICE_FUNCTION LambertBRDF (const RGB& reflectance) :
        m_reflectance (reflectance) {}

    // Get surface parameters like diffuse and specular reflectance, and roughness
    CUDA_DEVICE_FUNCTION void getSurfaceParameters (RGB* diffuseReflectance, RGB* specularReflectance, float* roughness) const
    {
        *diffuseReflectance = m_reflectance;           // Lambertian surfaces are purely diffuse
        *specularReflectance = RGB (0.0f, 0.0f, 0.0f); // No specular reflection
        *roughness = 1.0f;                             // Lambertian surfaces are completely rough
    }

    // Sample throughput for a given incoming direction
    // Also samples an outgoing direction based on input random variables uDir0 and uDir1
    CUDA_DEVICE_FUNCTION RGB sampleThroughput (
        const Vector3D& vGiven, float uDir0, float uDir1,
        Vector3D* vSampled, float* dirPDensity) const
    {
        *vSampled = cosineSampleHemisphere (uDir0, uDir1); // Sample direction in hemisphere
        *dirPDensity = vSampled->z / Pi;                   // Probability density for the sampled direction
        if (vGiven.z <= 0.0f)                              // Flip z if incoming direction is below the surface
            vSampled->z *= -1;
        return m_reflectance; // Return the reflectance value
    }

    // Evaluate the BRDF for a given incoming and outgoing direction
    CUDA_DEVICE_FUNCTION RGB evaluate (const Vector3D& vGiven, const Vector3D& vSampled) const
    {
        if (vGiven.z * vSampled.z > 0) // Both vectors should be on the same side of the surface
            return m_reflectance / Pi;
        else
            return RGB (0.0f, 0.0f, 0.0f); // Zero if vectors are on opposite sides
    }

    // Evaluate the PDF for the BRDF for given directions
    CUDA_DEVICE_FUNCTION float evaluatePDF (const Vector3D& vGiven, const Vector3D& vSampled) const
    {
        if (vGiven.z * vSampled.z > 0)
            return fabs (vSampled.z) / Pi;
        else
            return 0.0f; // Zero PDF if vectors are on opposite sides
    }

    // Estimate the directional-hemispherical reflectance for a given direction
    CUDA_DEVICE_FUNCTION RGB evaluateDHReflectanceEstimate (const Vector3D& vGiven) const
    {
        return m_reflectance; // For Lambertian, it's just the reflectance
    }
};

// This function is for computing the direct lighting on a surface point (shadingPoint)
// from an environment light source (lightSample) using a Lambertian BRDF (bsdf).
// It takes into account visibility, distances, and angles to compute the final light
// contribution at that point.
CUDA_DEVICE_FUNCTION CUDA_INLINE RGB computeDirectLightingFromEnvironment (
    const Point3D& shadingPoint, const Vector3D& vOutLocal, const ReferenceFrame& shadingFrame,
    const LambertBRDF& bsdf, const Shared::LightSample& lightSample)
{
    // Calculate the direction of the shadow ray
    Vector3D shadowRayDir = lightSample.atInfinity ? Vector3D (lightSample.position) : (lightSample.position - shadingPoint);

    // Calculate the distance squared and distance between the light and the shading point
    float dist2 = shadowRayDir.sqLength();
    float dist = std::sqrt (dist2);

    // Normalize the shadow ray direction
    shadowRayDir /= dist;

    // Convert shadow ray direction to local coordinate system
    Vector3D shadowRayDirLocal = shadingFrame.toLocal (shadowRayDir);

    // Compute the cosine of the angle between the light direction and light normal
    float lpCos = dot (-shadowRayDir, lightSample.normal);

    // Compute the cosine of the angle between shadow ray and normal at the shading point in local coords
    float spCos = shadowRayDirLocal.z;

    // Initialize visibility to 1 (completely visible)
    float visibility = 1.0f;

    // Set a high distance for lights at infinity
    if (lightSample.atInfinity)
        dist = 1e+10f;

    // Perform visibility ray tracing to check if the light is occluded
    Shared::VisibilityRayPayloadSignature::trace (
        plp.travHandle,
        shadingPoint.toNative(), shadowRayDir.toNative(), 0.0f, dist * 0.9999f, 0.0f,
        0xFF, OPTIX_RAY_FLAG_NONE,
        RayType::RayType_Visibility, Shared::NumRayTypes, RayType::RayType_Visibility,
        visibility);

    // If the point is visible and faces the light
    if (visibility > 0 && lpCos > 0)
    {
        // Calculate emittance assuming the light is a diffuse emitter
        RGB Le = lightSample.emittance / Pi;

        // Evaluate the Lambertian BRDF
        RGB fsValue = bsdf.evaluate (vOutLocal, shadowRayDirLocal);

        // Calculate the geometry term
        float G = lpCos * std::fabs (spCos) / dist2;

        // Final lighting contribution
        RGB ret = fsValue * Le * G;
        return ret;
    }
    else
    {
        // Return black if the point is not visible or does not face the light
        return RGB (0.0f, 0.0f, 0.0f);
    }
}

// This function samples an environmental light based on a set of
// random numbers (u0 and u1) and an importance map.It returns the
// sampled light direction, emittance, and some other attributes
// in lightSample.It also returns the probability density of the
// sampled area in areaPDensity.
CUDA_DEVICE_FUNCTION CUDA_INLINE void sampleEnviroLight (
    const Point3D& shadingPoint,
    float ul, bool sampleEnvLight, float u0, float u1,
    Shared::LightSample* lightSample, float* areaPDensity)
{
    CUtexObject texEmittance = 0;     // Texture object for light emittance
    RGB emittance (0.0f, 0.0f, 0.0f); // Light emittance color
    Point2D texCoord;                 // Texture coordinates

    float u, v;  // Parameters for sampling
    float uvPDF; // PDF for UV sampling

    // Sample the importance map to get UV coordinates and PDF
    plp.envLightImportanceMap.sample (u0, u1, &u, &v, &uvPDF);

    // Convert UV to spherical coordinates
    float phi = 2 * Pi * u;
    float theta = Pi * v;
    if (theta == 0.0f)
    {
        *areaPDensity = 0.0f;
        return;
    }

    // Apply rotation to the environment light
    float posPhi = phi - plp.envLightRotation;
    posPhi = posPhi - floorf (posPhi / (2 * Pi)) * 2 * Pi;

    // Convert spherical to Cartesian coordinates
    Vector3D direction = fromPolarYUp (posPhi, theta);
    Point3D position (direction.x, direction.y, direction.z);

    // Set light sample attributes
    lightSample->position = position;
    lightSample->atInfinity = true;
    lightSample->normal = Normal3D (-position);

    // Compute the area PDF
    *areaPDensity = uvPDF / (2 * Pi * Pi * std::sin (theta));

    // Retrieve the environment light texture
    texEmittance = plp.envLightTexture;

    // Set a base emittance value
    emittance = RGB (Pi * plp.envLightPowerCoeff);
    texCoord.x = u;
    texCoord.y = v;

    // If a texture is available, update emittance based on texture values
    if (texEmittance)
    {
        float4 texValue = tex2DLod<float4> (texEmittance, texCoord.x, texCoord.y, 0.0f);
        emittance *= RGB (texValue.x, texValue.y, texValue.z);
    }

    // Set the emittance in the light sample
    lightSample->emittance = emittance;
}

// Next Event Estimation (NEE) is a technique used in path tracing to improve
// the convergence of the rendered image. Instead of randomly bouncing rays around the scene,
// NEE takes a shortcut and directly samples a light source to check if it contributes to
// the illumination of a point.

// In a traditional path tracer, rays are shot from the camera and bounce around the scene
// until they hit a light source. This can take many bounces and lead to a noisy image.

// With NEE, when a ray hits a surface, the algorithm also sends a direct ray to a light source
// to see if it's visible from that point. This helps to quickly account for direct illumination,
// making the image converge faster and reducing noise.

// This function is for performing Next Event Estimation (NEE) in path tracing.
// It samples a light source, computes the direct lighting from that source,
// and combines it with the BRDF and visibility information.The function also
// uses Multiple Importance Sampling (MIS)
// to balance the contributions from the BRDF and the light source.
CUDA_DEVICE_FUNCTION CUDA_INLINE RGB performNextEventEstimation (
    const Point3D& shadingPoint, const Vector3D& vOutLocal, const ReferenceFrame& shadingFrame,
    const LambertBRDF& bsdf,
    PCG32RNG& rng)
{
    RGB ret (0.0f); // Initialize the return value

    // Generate a random number to select a light source
    float uLight = rng.getFloat0cTo1o();
    bool selectEnvLight = true;
    float probToSampleCurLightType = 1.0f; // Probability of sampling the current light type

    LightSample lightSample; // Sampled light information
    float areaPDensity;      // Area probability density
    // Sample the environmental light source
    sampleEnviroLight (
        shadingPoint,
        uLight, selectEnvLight, rng.getFloat0cTo1o(), rng.getFloat0cTo1o(),
        &lightSample, &areaPDensity);

    areaPDensity *= probToSampleCurLightType; // Update the area PDF with the light type selection probability

    float misWeight = 1.0f; // Multiple Importance Sampling (MIS) weight

    // Calculate the shadow ray direction
    Vector3D shadowRay = lightSample.atInfinity ? Vector3D (lightSample.position) : (lightSample.position - shadingPoint);
    float dist2 = shadowRay.sqLength();                   // Distance squared to the light
    shadowRay /= std::sqrt (dist2);                       // Normalize the shadow ray
    Vector3D vInLocal = shadingFrame.toLocal (shadowRay); // Convert to local coordinates

    // Calculate the cosine term and BSDF PDF
    float lpCos = std::fabs (dot (shadowRay, lightSample.normal));
    float bsdfPDensity = bsdf.evaluatePDF (vOutLocal, vInLocal) * lpCos / dist2; // BSDF PDF
    if (!isfinite (bsdfPDensity))                                                // Check for invalid values
        bsdfPDensity = 0.0f;

    // Calculate the light source PDF and MIS weight
    float lightPDensity = areaPDensity;
    misWeight = pow2 (lightPDensity) / (pow2 (bsdfPDensity) + pow2 (lightPDensity));

    // Compute the direct lighting contribution if the area PDF is positive
    if (areaPDensity > 0.0f)
        ret = computeDirectLightingFromEnvironment (
                  shadingPoint, vOutLocal, shadingFrame, bsdf, lightSample) *
              (misWeight / areaPDensity);

    return ret; // Return the final lighting contribution
}

// This function calculates various attributes of a surface point
// given its barycentric coordinates (b1, b2) and the index (primIndex)
// of the triangle it belongs to. It computes the world-space position,
// shading normal, texture coordinates, and so forth for this surface point.
// It also computes a hypothetical area PDF (hypAreaPDensity) that could
// be used in light sampling.
CUDA_DEVICE_FUNCTION CUDA_INLINE void computeSurfacePoint (
    const Shared::GeometryData& geomInst,
    uint32_t primIndex, float b1, float b2,
    const Point3D& referencePoint,
    Point3D* positionInWorld, Normal3D* shadingNormalInWorld, Vector3D* texCoord0DirInWorld,
    Normal3D* geometricNormalInWorld, Point2D* texCoord,
    float* hypAreaPDensity)
{
    // Fetch the vertices of the triangle given its index
    const Triangle& tri = geomInst.triangleBuffer[primIndex];
    const Vertex& v0 = geomInst.vertexBuffer[tri.index0];
    const Vertex& v1 = geomInst.vertexBuffer[tri.index1];
    const Vertex& v2 = geomInst.vertexBuffer[tri.index2];

    // Transform vertex positions to world space
    const Point3D p[3] = {
        transformPointFromObjectToWorldSpace (v0.position),
        transformPointFromObjectToWorldSpace (v1.position),
        transformPointFromObjectToWorldSpace (v2.position),
    };

    // Calculate barycentric coordinates
    float b0 = 1 - (b1 + b2);

    // Compute the position in world space using barycentric coordinates
    *positionInWorld = b0 * p[0] + b1 * p[1] + b2 * p[2];

    // Compute interpolated shading normal and texture direction
    Normal3D shadingNormal = b0 * v0.normal + b1 * v1.normal + b2 * v2.normal;
    Vector3D texCoord0Dir = b0 * v0.texCoord0Dir + b1 * v1.texCoord0Dir + b2 * v2.texCoord0Dir;

    // Compute geometric normal and area of the triangle
    Normal3D geometricNormal (cross (p[1] - p[0], p[2] - p[0]));
    float area = 0.5f * length (geometricNormal);

    // Compute the texture coordinates
    *texCoord = b0 * v0.texCoord + b1 * v1.texCoord + b2 * v2.texCoord;

    // Transform shading normal and texture direction to world space
    *shadingNormalInWorld = normalize (transformNormalFromObjectToWorldSpace (shadingNormal));
    *texCoord0DirInWorld = normalize (transformVectorFromObjectToWorldSpace (texCoord0Dir));
    *geometricNormalInWorld = normalize (geometricNormal);

    // Check for invalid normals and correct them
    if (!shadingNormalInWorld->allFinite())
    {
        *shadingNormalInWorld = Normal3D (0, 0, 1);
        *texCoord0DirInWorld = Vector3D (1, 0, 0);
    }

    // Check for invalid texture directions and correct them
    if (!texCoord0DirInWorld->allFinite())
    {
        Vector3D bitangent;
        makeCoordinateSystem (*shadingNormalInWorld, texCoord0DirInWorld, &bitangent);
    }

    // Compute the probability of sampling this light
    float lightProb = 1.0f;
    if (plp.envLightTexture && plp.enableEnvLight)
        lightProb *= (1 - probToSampleEnvLight);

    // Check for invalid probabilities
    if (!isfinite (lightProb))
    {
        *hypAreaPDensity = 0.0f;
        return;
    }

    // Compute the hypothetical area PDF
    *hypAreaPDensity = lightProb / area;
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

// Define the ray generating kernel for path tracing
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
#if 1
    if (isnan (colorResult.r) || isnan (colorResult.g) || isnan (colorResult.b))
    {
        // Add this line to print the payload.contribution values
        printf ("payload.contribution: %f, %f, %f\n", payload.contribution.r, payload.contribution.g, payload.contribution.b);
        colorResult = RGB (make_float3 (1000000.0f, 0.0f, 0.0f)); // super red
    }
    else if (isinf (colorResult.r) || isinf (colorResult.g) || isinf (colorResult.b))
    {
        printf ("payload.contribution: %f, %f, %f\n", payload.contribution.r, payload.contribution.g, payload.contribution.b);
        colorResult = RGB (make_float3 (0.0f, 1000000.0f, 0.0f)); // super green
    }
    else if (colorResult.r < 0.0f || colorResult.g < 0.0f || colorResult.b < 0.0f)
    {
        printf ("payload.contribution: %f, %f, %f\n", payload.contribution.r, payload.contribution.g, payload.contribution.b);
        colorResult = RGB (make_float3 (0.0f, 0.0f, 1000000.0f)); // super blue
    }
#endif
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
    if (payload->pathLength > 1) // coming off a surface
    {
        float uvPDF = plp.envLightImportanceMap.evaluatePDF (texCoord.x, texCoord.y);
        float hypAreaPDensity = uvPDF / (2 * Pi * Pi * std::sin (theta));
        // FIXME

        float lightPDensity =
            (plp.lightInstDist.integral() > 0.0f ? probToSampleEnvLight : 1.0f) *
            hypAreaPDensity;
        // FIXME
        // float bsdfPDensity = rwPayload->prevDirPDensity;
        float bsdfPDensity = 0.25f; // just guessing
        misWeight = pow2 (bsdfPDensity) / (pow2 (bsdfPDensity) + pow2 (lightPDensity));

        payload->contribution += payload->alpha * luminance * misWeight;
    }
    else
        payload->contribution = luminance;

    // Terminate the ray
    payload->terminate = true;
}

CUDA_DEVICE_KERNEL void RT_CH_NAME (shading)()
{
    // Retrieve material and geometry data from SBT (Shader Binding Table)
    auto sbtr = HitGroupSBTRecordData::get();
    const Shared::MaterialData& mat = sbtr.matData;
    const GeometryData& geom = sbtr.geomData;

    // Initialize random number generator and payload
    PCG32RNG rng;
    SearchRayPayload* payload;
    RGB* firstHitAlbedo;
    Normal3D* firstHitNormal;
    HitPointParams* hitPntParams;
    SearchRayPayloadSignature::get (&rng, &payload, &hitPntParams, &firstHitAlbedo, &firstHitNormal);

    // Retrieve ray origin in world coordinates
    const Point3D rayOrigin (optixGetWorldRayOrigin());

    // Get hit point parameters and compute various surface attributes
    auto hp = HitPointParameter::get();
    Point3D positionInWorld;
    Normal3D shadingNormalInWorld;
    Vector3D texCoord0DirInWorld;
    Normal3D geometricNormalInWorld;
    Point2D texCoord;
    float hypAreaPDensity;
    computeSurfacePoint (
        geom, hp.primIndex, hp.b1, hp.b2,
        rayOrigin,
        &positionInWorld, &shadingNormalInWorld, &texCoord0DirInWorld,
        &geometricNormalInWorld, &texCoord, &hypAreaPDensity);

    // Compute outgoing direction in world coordinates and check hit side
    Vector3D vOut = normalize (-Vector3D (optixGetWorldRayDirection()));
    float frontHit = dot (vOut, geometricNormalInWorld) >= 0.0f ? 1.0f : -1.0f;

    // Create a reference frame for shading based on shading normal
    ReferenceFrame shadingFrame (shadingNormalInWorld, texCoord0DirInWorld);

    // Offset the hit point along the normal to avoid self-intersection
    positionInWorld = offsetRayOrigin (positionInWorld, frontHit * geometricNormalInWorld);
    Vector3D vOutLocal = shadingFrame.toLocal (vOut);

    // Fetch or calculate albedo
    RGB albedo;
    if (mat.texture)
        albedo = RGB (getXYZ (tex2DLod<float4> (mat.texture, texCoord.x, texCoord.y, 0.0f)));
    else
        albedo = RGB (mat.albedo);

    // Create Lambertian BRDF
    LambertBRDF bsdf (albedo);

    // Perform Next Event Estimation for direct lighting
    payload->contribution += payload->alpha * performNextEventEstimation (
                                                  positionInWorld, vOutLocal, shadingFrame, bsdf, rng);

    // Generate the next ray for path tracing
    Vector3D vInLocal;
    float dirPDensity;
    payload->alpha *= bsdf.sampleThroughput (
        vOutLocal, rng.getFloat0cTo1o(), rng.getFloat0cTo1o(),
        &vInLocal, &dirPDensity);
    Vector3D vIn = shadingFrame.fromLocal (vInLocal);

    // Store hit point data for subsequent usage
    hitPntParams->normalInWorld = shadingNormalInWorld;
    if (payload->pathLength == 1)
    {
        *firstHitAlbedo = albedo;
        *firstHitNormal = shadingNormalInWorld;
    }

    // Update payload with new ray data
    payload->origin = positionInWorld;
    payload->direction = vIn;
    payload->terminate = false;

    // Finalize payload
    SearchRayPayloadSignature::set (&rng, nullptr, nullptr, nullptr, nullptr);
}

CUDA_DEVICE_KERNEL void RT_AH_NAME (visibility)()
{
    float visibility = 0.0f;
    VisibilityRayPayloadSignature::set (&visibility);

    optixTerminateRay();
}
