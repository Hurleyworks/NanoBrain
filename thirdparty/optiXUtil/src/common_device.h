#pragma once

// taken from Shocker GfxExp
// https://github.com/shocker-0x15/GfxExp

#include "common_shared.h"

#define V2FMT "%g, %g"
#define V3FMT "%g, %g, %g"
#define V4FMT "%g, %g, %g"
#define v2print(v) (v).x, (v).y
#define v3print(v) (v).x, (v).y, (v).z
#define v4print(v) (v).x, (v).y, (v).z, (v).w



static constexpr float Pi = 3.14159265358979323846f;
static constexpr float RayEpsilon = 1e-4;



// ( 0, 0,  1) <=> phi:      0
// (-1, 0,  0) <=> phi: 1/2 pi
// ( 0, 0, -1) <=> phi:   1 pi
// ( 1, 0,  0) <=> phi: 3/2 pi
CUDA_DEVICE_FUNCTION CUDA_INLINE Vector3D fromPolarYUp(float phi, float theta) {
    float sinPhi, cosPhi;
    float sinTheta, cosTheta;
    sincosf(phi, &sinPhi, &cosPhi);
    sincosf(theta, &sinTheta, &cosTheta);
    return Vector3D(-sinPhi * sinTheta, cosTheta, cosPhi * sinTheta);
}
CUDA_DEVICE_FUNCTION CUDA_INLINE void toPolarYUp(const Vector3D &v, float* phi, float* theta) {
    *theta = std::acos(min(max(v.y, -1.0f), 1.0f));
    *phi = std::fmod(std::atan2(-v.x, v.z) + 2 * Pi,
                     2 * Pi);
}

CUDA_DEVICE_FUNCTION CUDA_INLINE Vector3D halfVector(const Vector3D &a, const Vector3D &b) {
    return normalize(a + b);
}

template <bool isNormalA, bool isNormalB>
CUDA_DEVICE_FUNCTION CUDA_INLINE float absDot(
    const Vector3D_T<float, isNormalA> &a, const Vector3D_T<float, isNormalB> &b) {
    return std::fabs(dot(a, b));
}

CUDA_DEVICE_FUNCTION CUDA_INLINE void makeCoordinateSystem(
    const Normal3D &normal, Vector3D* tangent, Vector3D* bitangent) {
    float sign = normal.z >= 0 ? 1 : -1;
    const float a = -1 / (sign + normal.z);
    const float b = normal.x * normal.y * a;
    *tangent = Vector3D(1 + sign * normal.x * normal.x * a, sign * b, -sign * normal.x);
    *bitangent = Vector3D(b, sign + normal.y * normal.y * a, -normal.y);
}

// JP: 自己交叉回避のためにレイの原点にオフセットを付加する。
// EN: Add an offset to a ray origin to avoid self-intersection.
CUDA_DEVICE_FUNCTION CUDA_INLINE Point3D offsetRayOriginNaive(
    const Point3D &p, const Normal3D &geometricNormal) {
    return p + RayEpsilon * geometricNormal;
}

// Reference:
// Chapter 6. A Fast and Robust Method for Avoiding Self-Intersection, Ray Tracing Gems, 2019
CUDA_DEVICE_FUNCTION CUDA_INLINE Point3D offsetRayOrigin(
    const Point3D &p, const Normal3D &geometricNormal) {
    constexpr float kOrigin = 1.0f / 32.0f;
    constexpr float kFloatScale = 1.0f / 65536.0f;
    constexpr float kIntScale = 256.0f;

    int32_t offsetInInt[] = {
        static_cast<int32_t>(kIntScale * geometricNormal.x),
        static_cast<int32_t>(kIntScale * geometricNormal.y),
        static_cast<int32_t>(kIntScale * geometricNormal.z)
    };

    // JP: 数学的な衝突点の座標と、実際の座標の誤差は原点からの距離に比例する。
    //     intとしてオフセットを加えることでスケール非依存に適切なオフセットを加えることができる。
    // EN: The error of the actual coorinates of the intersection point to the mathematical one is proportional to the distance to the origin.
    //     Applying the offset as int makes applying appropriate scale invariant amount of offset possible.
    Point3D newP1(__int_as_float(__float_as_int(p.x) + (p.x < 0 ? -1 : 1) * offsetInInt[0]),
                  __int_as_float(__float_as_int(p.y) + (p.y < 0 ? -1 : 1) * offsetInInt[1]),
                  __int_as_float(__float_as_int(p.z) + (p.z < 0 ? -1 : 1) * offsetInInt[2]));

    // JP: 原点に近い場所では、原点からの距離に依存せず一定の誤差が残るため別処理が必要。
    // EN: A constant amount of error remains near the origin independent of the distance to the origin so we need handle it separately.
    Point3D newP2 = p + kFloatScale * geometricNormal;

    return Point3D(std::fabs(p.x) < kOrigin ? newP2.x : newP1.x,
                   std::fabs(p.y) < kOrigin ? newP2.y : newP1.y,
                   std::fabs(p.z) < kOrigin ? newP2.z : newP1.z);
}

CUDA_DEVICE_FUNCTION CUDA_INLINE Point2D adjustTexCoord(
    shared::TexDimInfo dimInfo, const Point2D &texCoord) {
    Point2D mTexCoord = texCoord;
    if (dimInfo.isNonPowerOfTwo && dimInfo.isBCTexture) {
        uint32_t bcWidth = (dimInfo.dimX + 3) / 4 * 4;
        uint32_t bcHeight = (dimInfo.dimY + 3) / 4 * 4;
        mTexCoord.x *= static_cast<float>(dimInfo.dimX) / bcWidth;
        mTexCoord.y *= static_cast<float>(dimInfo.dimY) / bcHeight;
    }
    return mTexCoord;
}

template <typename T>
CUDA_DEVICE_FUNCTION CUDA_INLINE T sample(
    CUtexObject texture, shared::TexDimInfo dimInfo, const Point2D &texCoord, float mipLevel) {
    Point2D mTexCoord = adjustTexCoord(dimInfo, texCoord);
    return tex2DLod<T>(texture, mTexCoord.x, mTexCoord.y, mipLevel);
}

struct ReferenceFrame {
    Vector3D tangent;
    Vector3D bitangent;
    Normal3D normal;

    CUDA_DEVICE_FUNCTION ReferenceFrame() {}
    CUDA_DEVICE_FUNCTION ReferenceFrame(
        const Vector3D &_tangent, const Vector3D &_bitangent, const Normal3D &_normal) :
        tangent(_tangent), bitangent(_bitangent), normal(_normal) {}
    CUDA_DEVICE_FUNCTION ReferenceFrame(const Normal3D &_normal) : normal(_normal) {
        makeCoordinateSystem(normal, &tangent, &bitangent);
    }
    CUDA_DEVICE_FUNCTION ReferenceFrame(const Normal3D &_normal, const Vector3D &_tangent) :
        tangent(_tangent), normal(_normal) {
        bitangent = cross(normal, tangent);
    }

    CUDA_DEVICE_FUNCTION Vector3D toLocal(const Vector3D &v) const {
        return Vector3D(dot(tangent, v), dot(bitangent, v), dot(normal, v));
    }
    CUDA_DEVICE_FUNCTION Vector3D fromLocal(const Vector3D &v) const {
        return Vector3D(dot(Vector3D(tangent.x, bitangent.x, normal.x), v),
                        dot(Vector3D(tangent.y, bitangent.y, normal.y), v),
                        dot(Vector3D(tangent.z, bitangent.z, normal.z), v));
    }
};

CUDA_DEVICE_FUNCTION CUDA_INLINE void applyBumpMapping(
    const Normal3D &modNormalInTF, ReferenceFrame* frameToModify) {
    // JP: 法線から回転軸と回転角(、Quaternion)を求めて対応する接平面ベクトルを求める。
    // EN: calculate a rotating axis and an angle (and quaternion) from the normal then calculate corresponding tangential vectors.
    float projLength = std::sqrt(modNormalInTF.x * modNormalInTF.x + modNormalInTF.y * modNormalInTF.y);
    if (projLength < 1e-3f)
        return;
    float tiltAngle = std::atan(projLength / modNormalInTF.z);
    float qSin, qCos;
    sincosf(tiltAngle / 2, &qSin, &qCos);
    float qX = (-modNormalInTF.y / projLength) * qSin;
    float qY = (modNormalInTF.x / projLength) * qSin;
    float qW = qCos;
    Vector3D modTangentInTF(1 - 2 * qY * qY, 2 * qX * qY, -2 * qY * qW);
    Vector3D modBitangentInTF(2 * qX * qY, 1 - 2 * qX * qX, 2 * qX * qW);

    Matrix3x3 matTFtoW(
        frameToModify->tangent,
        frameToModify->bitangent,
        Vector3D(frameToModify->normal));
    ReferenceFrame bumpShadingFrame(
        matTFtoW * modTangentInTF,
        matTFtoW * modBitangentInTF,
        matTFtoW * modNormalInTF);

    *frameToModify = bumpShadingFrame;
}

#if 0
RT_CALLABLE_PROGRAM Normal3D RT_DC_NAME(readModifiedNormalFromNormalMap)
(CUtexObject texture, shared::TexDimInfo dimInfo, Point2D texCoord, float mipLevel) {
    float4 texValue = sample<float4>(texture, dimInfo, texCoord, mipLevel);
    Normal3D modLocalNormal(getXYZ(texValue));
    modLocalNormal = 2.0f * modLocalNormal - Normal3D(1.0f);
    if (dimInfo.isLeftHanded)
        modLocalNormal.y *= -1; // DirectX convention
    return modLocalNormal;
}
CUDA_DECLARE_CALLABLE_PROGRAM_POINTER(readModifiedNormalFromNormalMap);

RT_CALLABLE_PROGRAM Normal3D RT_DC_NAME(readModifiedNormalFromNormalMap2ch)
(CUtexObject texture, shared::TexDimInfo dimInfo, Point2D texCoord, float mipLevel) {
    float2 texValue = sample<float2>(texture, dimInfo, texCoord, mipLevel);
    Vector2D xy(texValue.x, texValue.y);
    xy = 2.0f * xy - Vector2D(1.0f);
    float z = std::sqrt(1.0f - pow2(xy.x) - pow2(xy.y));
    Normal3D modLocalNormal(xy.x, xy.y, z);
    if (dimInfo.isLeftHanded)
        modLocalNormal.y *= -1; // DirectX convention
    return modLocalNormal;
}
CUDA_DECLARE_CALLABLE_PROGRAM_POINTER(readModifiedNormalFromNormalMap2ch);

RT_CALLABLE_PROGRAM Normal3D RT_DC_NAME(readModifiedNormalFromHeightMap)
(CUtexObject texture, shared::TexDimInfo dimInfo, Point2D texCoord) {
    if (dimInfo.isNonPowerOfTwo && dimInfo.isBCTexture) {
        uint32_t bcWidth = (dimInfo.dimX + 3) / 4 * 4;
        uint32_t bcHeight = (dimInfo.dimY + 3) / 4 * 4;
        texCoord.x *= static_cast<float>(dimInfo.dimX) / bcWidth;
        texCoord.y *= static_cast<float>(dimInfo.dimY) / bcHeight;
    }
    float4 heightValues = tex2Dgather<float4>(texture, texCoord.x, texCoord.y, 0);
    constexpr float coeff = (5.0f / 1024);
    uint32_t width = dimInfo.dimX;
    uint32_t height = dimInfo.dimY;
    float dhdu = (coeff * width) * (heightValues.y - heightValues.x);
    float dhdv = (coeff * height) * (heightValues.x - heightValues.w);
    Normal3D modLocalNormal = normalize(Normal3D(-dhdu, dhdv, 1));
    return modLocalNormal;
}
CUDA_DECLARE_CALLABLE_PROGRAM_POINTER(readModifiedNormalFromHeightMap);
#endif


CUDA_DEVICE_FUNCTION CUDA_INLINE void concentricSampleDisk(float u0, float u1, float* dx, float* dy) {
    float r, theta;
    float sx = 2 * u0 - 1;
    float sy = 2 * u1 - 1;

    if (sx == 0 && sy == 0) {
        *dx = 0;
        *dy = 0;
        return;
    }
    if (sx >= -sy) { // region 1 or 2
        if (sx > sy) { // region 1
            r = sx;
            theta = sy / sx;
        }
        else { // region 2
            r = sy;
            theta = 2 - sx / sy;
        }
    }
    else { // region 3 or 4
        if (sx > sy) {/// region 4
            r = -sy;
            theta = 6 + sx / sy;
        }
        else {// region 3
            r = -sx;
            theta = 4 + sy / sx;
        }
    }
    theta *= Pi / 4;
    *dx = r * cos(theta);
    *dy = r * sin(theta);
}

CUDA_DEVICE_FUNCTION CUDA_INLINE Vector3D cosineSampleHemisphere(float u0, float u1) {
    float x, y;
    concentricSampleDisk(u0, u1, &x, &y);
    return Vector3D(x, y, std::sqrt(std::fmax(0.0f, 1.0f - x * x - y * y)));
}

CUDA_DEVICE_FUNCTION CUDA_INLINE Point3D transformPointFromObjectToWorldSpace(const Point3D &p) {
    float3 xfmP = optixTransformPointFromObjectToWorldSpace(make_float3(p.x, p.y, p.z));
    return Point3D(xfmP.x, xfmP.y, xfmP.z);
}

CUDA_DEVICE_FUNCTION CUDA_INLINE Vector3D transformVectorFromObjectToWorldSpace(const Vector3D &v) {
    float3 xfmV = optixTransformVectorFromObjectToWorldSpace(make_float3(v.x, v.y, v.z));
    return Vector3D(xfmV.x, xfmV.y, xfmV.z);
}

CUDA_DEVICE_FUNCTION CUDA_INLINE Normal3D transformNormalFromObjectToWorldSpace(const Normal3D &n) {
    float3 xfmN = optixTransformNormalFromObjectToWorldSpace(make_float3(n.x, n.y, n.z));
    return Normal3D(xfmN.x, xfmN.y, xfmN.z);
}
