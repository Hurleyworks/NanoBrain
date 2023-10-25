﻿#pragma once

// taken from OptiX_Utility
// https://github.com/shocker-0x15/OptiX_Utility/blob/master/LICENSE.md

// Platform defines
#if defined(_WIN32) || defined(_WIN64)
#define HP_Platform_Windows
#if defined(_MSC_VER)
#define HP_Platform_Windows_MSVC
#endif
#elif defined(__APPLE__)
#define HP_Platform_macOS
#endif

#ifdef _DEBUG
#define ENABLE_ASSERT
#define DEBUG_SELECT(A, B) A
#else
#define DEBUG_SELECT(A, B) B
#endif

#if defined(HP_Platform_Windows_MSVC)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef near
#undef far
#undef RGB
#endif

// #includes
#if defined(__CUDA_ARCH__)
#else
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cmath>

#if __cplusplus >= 202002L
#include <numbers>
#endif
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <thread>
#include <chrono>
#include <variant>

#include <immintrin.h>

// #   include "stopwatch.h"
#endif

#include <optixu_on_cudau.h>

#ifdef HP_Platform_Windows_MSVC
#if defined(__CUDA_ARCH__)
#define devPrintf(fmt, ...) printf (fmt, ##__VA_ARGS__);
#else
void devPrintf (const char* fmt, ...);
#endif
#else
#define devPrintf(fmt, ...) printf (fmt, ##__VA_ARGS__);
#endif

#ifdef ENABLE_ASSERT
#if defined(__CUDA_ARCH__)
#define Assert(expr, fmt, ...) \
    do \
    { \
        if (!(expr)) \
        { \
            devPrintf ("%s @%s: %u:\n", #expr, __FILE__, __LINE__); \
            devPrintf (fmt "\n", ##__VA_ARGS__); \
            assert (false); \
        } \
    } while (0)
#else
#define Assert(expr, fmt, ...) \
    do \
    { \
        if (!(expr)) \
        { \
            devPrintf ("%s @%s: %u:\n", #expr, __FILE__, __LINE__); \
            devPrintf (fmt "\n", ##__VA_ARGS__); \
            abort(); \
        } \
    } while (0)
#endif
#else
#define Assert(expr, fmt, ...)
#endif

#define Assert_ShouldNotBeCalled() Assert (false, "Should not be called!")
#define Assert_NotImplemented() Assert (false, "Not implemented yet!")

template <typename T, size_t size>
CUDA_COMMON_FUNCTION CUDA_INLINE constexpr size_t lengthof (const T (&array)[size])
{
    return size;
}

namespace shared
{
    template <typename T>
    CUDA_COMMON_FUNCTION CUDA_INLINE constexpr T min (const T& a, const T& b)
    {
        return b < a ? b : a;
    }
    template <typename T>
    CUDA_COMMON_FUNCTION CUDA_INLINE constexpr T max (const T& a, const T& b)
    {
        return b > a ? b : a;
    }
    template <typename T>
    CUDA_COMMON_FUNCTION CUDA_INLINE constexpr T clamp (const T& v, const T& minv, const T& maxv)
    {
        return min (max (v, minv), maxv);
    }
} // namespace shared

#if __CUDA_ARCH__ < 600
#define atomicOr_block atomicOr
#define atomicAnd_block atomicAnd
#define atomicAdd_block atomicAdd
#define atomicMin_block atomicMin
#define atomicMax_block atomicMax
#endif

template <typename T>
CUDA_COMMON_FUNCTION CUDA_INLINE T alignUp (T value, uint32_t alignment)
{
    return (value + alignment - 1) / alignment * alignment;
}

CUDA_COMMON_FUNCTION CUDA_INLINE uint32_t tzcnt (uint32_t x)
{
#if defined(__CUDA_ARCH__)
    return __clz (__brev (x));
#else
    return _tzcnt_u32 (x);
#endif
}

CUDA_COMMON_FUNCTION CUDA_INLINE uint32_t lzcnt (uint32_t x)
{
#if defined(__CUDA_ARCH__)
    return __clz (x);
#else
    return _lzcnt_u32 (x);
#endif
}

CUDA_COMMON_FUNCTION CUDA_INLINE int32_t popcnt (uint32_t x)
{
#if defined(__CUDA_ARCH__)
    return __popc (x);
#else
    return _mm_popcnt_u32 (x);
#endif
}

//     0: 0
//     1: 0
//  2- 3: 1
//  4- 7: 2
//  8-15: 3
// 16-31: 4
// ...
CUDA_COMMON_FUNCTION CUDA_INLINE uint32_t prevPowOf2Exponent (uint32_t x)
{
    if (x == 0)
        return 0;
    return 31 - lzcnt (x);
}

//    0: 0
//    1: 0
//    2: 1
// 3- 4: 2
// 5- 8: 3
// 9-16: 4
// ...
CUDA_COMMON_FUNCTION CUDA_INLINE uint32_t nextPowOf2Exponent (uint32_t x)
{
    if (x == 0)
        return 0;
    return 32 - lzcnt (x - 1);
}

//     0: 0
//     1: 1
//  2- 3: 2
//  4- 7: 4
//  8-15: 8
// 16-31: 16
// ...
CUDA_COMMON_FUNCTION CUDA_INLINE uint32_t prevPowerOf2 (uint32_t x)
{
    if (x == 0)
        return 0;
    return 1 << prevPowOf2Exponent (x);
}

//    0: 0
//    1: 1
//    2: 2
// 3- 4: 4
// 5- 8: 8
// 9-16: 16
// ...
CUDA_COMMON_FUNCTION CUDA_INLINE uint32_t nextPowerOf2 (uint32_t x)
{
    if (x == 0)
        return 0;
    return 1 << nextPowOf2Exponent (x);
}

template <typename IntType>
CUDA_COMMON_FUNCTION CUDA_INLINE constexpr IntType nextMultiplesForPowOf2 (IntType x, uint32_t exponent)
{
    IntType mask = (1 << exponent) - 1;
    return (x + mask) & ~mask;
}

template <typename IntType>
CUDA_COMMON_FUNCTION CUDA_INLINE constexpr IntType nextMultiplierForPowOf2 (IntType x, uint32_t exponent)
{
    return nextMultiplesForPowOf2 (x, exponent) >> exponent;
}

CUDA_COMMON_FUNCTION CUDA_INLINE uint32_t nthSetBit (uint32_t value, int32_t n)
{
    uint32_t idx = 0;
    int32_t count;
    if (n >= popcnt (value))
        return 0xFFFFFFFF;

    for (uint32_t width = 16; width >= 1; width >>= 1)
    {
        if (value == 0)
            return 0xFFFFFFFF;

        uint32_t mask = (1 << width) - 1;
        count = popcnt (value & mask);
        if (n >= count)
        {
            value >>= width;
            n -= count;
            idx += width;
        }
    }

    return idx;
}

template <typename T>
CUDA_COMMON_FUNCTION CUDA_INLINE T pow2 (const T& x)
{
    return x * x;
}

template <typename T>
CUDA_COMMON_FUNCTION CUDA_INLINE T pow3 (const T& x)
{
    return x * x * x;
}

// JP: CUDAビルトインに対応する型・関数をホスト側で定義しておく。
// EN: Define types and functions on the host corresponding to CUDA built-ins.
#if !defined(__CUDACC__) || defined(OPTIXU_Platform_CodeCompletion)
struct alignas (8) int2
{
    int32_t x, y;
    constexpr int2 (int32_t v = 0) :
        x (v), y (v) {}
    constexpr int2 (int32_t xx, int32_t yy) :
        x (xx), y (yy) {}
};
inline constexpr int2 make_int2 (int32_t x, int32_t y)
{
    return int2 (x, y);
}
struct int3
{
    int32_t x, y, z;
    constexpr int3 (int32_t v = 0) :
        x (v), y (v), z (v) {}
    constexpr int3 (int32_t xx, int32_t yy, int32_t zz) :
        x (xx), y (yy), z (zz) {}
};
inline constexpr int3 make_int3 (int32_t x, int32_t y, int32_t z)
{
    return int3 (x, y, z);
}
struct alignas (16) int4
{
    int32_t x, y, z, w;
    constexpr int4 (int32_t v = 0) :
        x (v), y (v), z (v), w (v) {}
    constexpr int4 (int32_t xx, int32_t yy, int32_t zz, int32_t ww) :
        x (xx), y (yy), z (zz), w (ww) {}
};
inline constexpr int4 make_int4 (int32_t x, int32_t y, int32_t z, int32_t w)
{
    return int4 (x, y, z, w);
}
struct alignas (8) uint2
{
    uint32_t x, y;
    constexpr uint2 (uint32_t v = 0) :
        x (v), y (v) {}
    constexpr uint2 (uint32_t xx, uint32_t yy) :
        x (xx), y (yy) {}
};
inline constexpr uint2 make_uint2 (uint32_t x, uint32_t y)
{
    return uint2 (x, y);
}
struct uint3
{
    uint32_t x, y, z;
    constexpr uint3 (uint32_t v = 0) :
        x (v), y (v), z (v) {}
    constexpr uint3 (uint32_t xx, uint32_t yy, uint32_t zz) :
        x (xx), y (yy), z (zz) {}
};
inline constexpr uint3 make_uint3 (uint32_t x, uint32_t y, uint32_t z)
{
    return uint3 (x, y, z);
}
struct uint4
{
    uint32_t x, y, z, w;
    constexpr uint4 (uint32_t v = 0) :
        x (v), y (v), z (v), w (v) {}
    constexpr uint4 (uint32_t xx, uint32_t yy, uint32_t zz, uint32_t ww) :
        x (xx), y (yy), z (zz), w (ww) {}
};
inline constexpr uint4 make_uint4 (uint32_t x, uint32_t y, uint32_t z, uint32_t w)
{
    return uint4 (x, y, z, w);
}
struct alignas (8) float2
{
    float x, y;
    constexpr float2 (float v = 0) :
        x (v), y (v) {}
    constexpr float2 (float xx, float yy) :
        x (xx), y (yy) {}
};
inline float2 make_float2 (float x, float y)
{
    return float2 (x, y);
}
struct float3
{
    float x, y, z;
    constexpr float3 (float v = 0) :
        x (v), y (v), z (v) {}
    constexpr float3 (float xx, float yy, float zz) :
        x (xx), y (yy), z (zz) {}
};
inline constexpr float3 make_float3 (float x, float y, float z)
{
    return float3 (x, y, z);
}
struct alignas (16) float4
{
    float x, y, z, w;
    constexpr float4 (float v = 0) :
        x (v), y (v), z (v), w (v) {}
    constexpr float4 (float xx, float yy, float zz, float ww) :
        x (xx), y (yy), z (zz), w (ww) {}
};
inline constexpr float4 make_float4 (float x, float y, float z, float w)
{
    return float4 (x, y, z, w);
}
#endif

CUDA_COMMON_FUNCTION CUDA_INLINE float3 getXYZ (const float4& v)
{
    return make_float3 (v.x, v.y, v.z);
}

CUDA_COMMON_FUNCTION CUDA_INLINE int2 make_int2 (const float2& v)
{
    return make_int2 (static_cast<int32_t> (v.x), static_cast<int32_t> (v.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2 make_int2 (const int3& v)
{
    return make_int2 (v.x, v.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2 make_int2 (const uint3& v)
{
    return make_int2 (static_cast<int32_t> (v.x), static_cast<int32_t> (v.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator== (const int2& v0, const int2& v1)
{
    return v0.x == v1.x && v0.y == v1.y;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator!= (const int2& v0, const int2& v1)
{
    return v0.x != v1.x || v0.y != v1.y;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator== (const int2& v0, const uint2& v1)
{
    return v0.x == v1.x && v0.y == v1.y;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator!= (const int2& v0, const uint2& v1)
{
    return v0.x != v1.x || v0.y != v1.y;
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator+ (const int2& v0, const uint2& v1)
{
    return make_uint2 (v0.x + v1.x, v0.y + v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2 operator+ (const int2& v0, const int2& v1)
{
    return make_int2 (v0.x + v1.x, v0.y + v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2 operator* (const int2& v0, const int2& v1)
{
    return make_int2 (v0.x * v1.x, v0.y * v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2 operator* (uint32_t s, const int2& v)
{
    return make_int2 (s * v.x, s * v.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2 operator* (const int2& v, uint32_t s)
{
    return make_int2 (s * v.x, s * v.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2& operator*= (int2& v0, const int2& v1)
{
    v0.x *= v1.x;
    v0.y *= v1.y;
    return v0;
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2& operator*= (int2& v, uint32_t s)
{
    v.x *= s;
    v.y *= s;
    return v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2 operator/ (const int2& v0, const int2& v1)
{
    return make_int2 (v0.x / v1.x, v0.y / v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2 operator/ (const int2& v, uint32_t s)
{
    return make_int2 (v.x / s, v.y / s);
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator/ (const int2& v0, const uint2& v1)
{
    return make_uint2 (v0.x / v1.x, v0.y / v1.y);
}

CUDA_COMMON_FUNCTION CUDA_INLINE uint2 make_uint2 (const float2& v)
{
    return make_uint2 (static_cast<uint32_t> (v.x), static_cast<uint32_t> (v.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 make_uint2 (const int3& v)
{
    return make_uint2 (static_cast<uint32_t> (v.x), static_cast<uint32_t> (v.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 make_uint2 (const uint3& v)
{
    return make_uint2 (v.x, v.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator== (const uint2& v0, const uint2& v1)
{
    return v0.x == v1.x && v0.y == v1.y;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator!= (const uint2& v0, const uint2& v1)
{
    return v0.x != v1.x || v0.y != v1.y;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator== (const uint2& v0, const int2& v1)
{
    return v0.x == v1.x && v0.y == v1.y;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator!= (const uint2& v0, const int2& v1)
{
    return v0.x != v1.x || v0.y != v1.y;
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator+ (const uint2& v0, const uint2& v1)
{
    return make_uint2 (v0.x + v1.x, v0.y + v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2& operator+= (uint2& v, uint32_t s)
{
    v.x += s;
    v.y += s;
    return v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator- (const uint2& v, uint32_t s)
{
    return make_uint2 (v.x - s, v.y - s);
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2& operator-= (uint2& v, uint32_t s)
{
    v.x -= s;
    v.y -= s;
    return v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator* (const uint2& v0, const uint2& v1)
{
    return make_uint2 (v0.x * v1.x, v0.y * v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator* (float s, const uint2& v)
{
    return make_uint2 (static_cast<uint32_t> (s * v.x), static_cast<uint32_t> (s * v.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator* (const uint2& v, float s)
{
    return make_uint2 (static_cast<uint32_t> (s * v.x), static_cast<uint32_t> (s * v.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2& operator*= (uint2& v0, const uint2& v1)
{
    v0.x *= v1.x;
    v0.y *= v1.y;
    return v0;
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2& operator*= (uint2& v, uint32_t s)
{
    v.x *= s;
    v.y *= s;
    return v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator/ (const uint2& v0, const uint2& v1)
{
    return make_uint2 (v0.x / v1.x, v0.y / v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator/ (const uint2& v0, const int2& v1)
{
    return make_uint2 (v0.x / v1.x, v0.y / v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator/ (const uint2& v, uint32_t s)
{
    return make_uint2 (v.x / s, v.y / s);
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2& operator/= (uint2& v, uint32_t s)
{
    v.x /= s;
    v.y /= s;
    return v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator% (const uint2& v0, const uint2& v1)
{
    return make_uint2 (v0.x % v1.x, v0.y % v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator<< (const uint2& v, uint32_t s)
{
    return make_uint2 (v.x << s, v.y << s);
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2& operator<<= (uint2& v, uint32_t s)
{
    v = v << s;
    return v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 operator>> (const uint2& v, uint32_t s)
{
    return make_uint2 (v.x >> s, v.y >> s);
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2& operator>>= (uint2& v, uint32_t s)
{
    v = v >> s;
    return v;
}

CUDA_COMMON_FUNCTION CUDA_INLINE float2 make_float2 (float v)
{
    return make_float2 (v, v);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 make_float2 (const int2& v)
{
    return make_float2 (static_cast<float> (v.x), static_cast<float> (v.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 make_float2 (const uint2& v)
{
    return make_float2 (static_cast<float> (v.x), static_cast<float> (v.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator== (const float2& v0, const float2& v1)
{
    return v0.x == v1.x && v0.y == v1.y;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator!= (const float2& v0, const float2& v1)
{
    return v0.x != v1.x || v0.y != v1.y;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator- (const float2& v)
{
    return make_float2 (-v.x, -v.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator+ (const float2& v0, const float2& v1)
{
    return make_float2 (v0.x + v1.x, v0.y + v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator- (const float2& v0, const float2& v1)
{
    return make_float2 (v0.x - v1.x, v0.y - v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator* (const float2& v0, const float2& v1)
{
    return make_float2 (v0.x * v1.x, v0.y * v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator* (float s, const float2& v)
{
    return make_float2 (s * v.x, s * v.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator* (const float2& v, float s)
{
    return make_float2 (s * v.x, s * v.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2& operator*= (float2& v, float s)
{
    v = v * s;
    return v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator* (const int2& v0, const float2& v1)
{
    return make_float2 (v0.x * v1.x, v0.y * v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator* (const float2& v0, const int2& v1)
{
    return make_float2 (v0.x * v1.x, v0.y * v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator/ (const float2& v0, const float2& v1)
{
    return make_float2 (v0.x / v1.x, v0.y / v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator/ (const float2& v0, const int2& v1)
{
    return make_float2 (v0.x / v1.x, v0.y / v1.y);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 operator/ (const float2& v, float s)
{
    float r = 1 / s;
    return r * v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2& operator/= (float2& v, float s)
{
    v = v / s;
    return v;
}

CUDA_COMMON_FUNCTION CUDA_INLINE float3 make_float3 (float v)
{
    return make_float3 (v, v, v);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 make_float3 (const float4& v)
{
    return make_float3 (v.x, v.y, v.z);
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator== (const float3& v0, const float3& v1)
{
    return v0.x == v1.x && v0.y == v1.y && v0.z == v1.z;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator!= (const float3& v0, const float3& v1)
{
    return v0.x != v1.x || v0.y != v1.y || v0.z != v1.z;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 operator- (const float3& v)
{
    return make_float3 (-v.x, -v.y, -v.z);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 operator+ (const float3& v0, const float3& v1)
{
    return make_float3 (v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3& operator+= (float3& v0, const float3& v1)
{
    v0.x += v1.x;
    v0.y += v1.y;
    v0.z += v1.z;
    return v0;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 operator- (const float3& v0, const float3& v1)
{
    return make_float3 (v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3& operator-= (float3& v0, const float3& v1)
{
    v0.x -= v1.x;
    v0.y -= v1.y;
    v0.z -= v1.z;
    return v0;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 operator* (const float3& v0, const float3& v1)
{
    return make_float3 (v0.x * v1.x, v0.y * v1.y, v0.z * v1.z);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 operator* (float s, const float3& v)
{
    return make_float3 (s * v.x, s * v.y, s * v.z);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 operator* (const float3& v, float s)
{
    return make_float3 (s * v.x, s * v.y, s * v.z);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3& operator*= (float3& v0, const float3& v1)
{
    v0.x *= v1.x;
    v0.y *= v1.y;
    v0.z *= v1.z;
    return v0;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3& operator*= (float3& v, float s)
{
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 operator/ (const float3& v0, const float3& v1)
{
    return make_float3 (v0.x / v1.x, v0.y / v1.y, v0.z / v1.z);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 operator/ (const float3& v, float s)
{
    float r = 1 / s;
    return r * v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 safeDivide (const float3& v0, const float3& v1)
{
    return make_float3 (
        v1.x != 0.0f ? v0.x / v1.x : 0.0f,
        v1.y != 0.0f ? v0.y / v1.y : 0.0f,
        v1.z != 0.0f ? v0.z / v1.z : 0.0f);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 safeDivide (const float3& v, float d)
{
    return d != 0.0f ? (v / d) : make_float3 (0.0f);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3& operator/= (float3& v, float s)
{
    float r = 1 / s;
    return v *= r;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool allFinite (const float3& v)
{
#if !defined(__CUDA_ARCH__)
    using std::isfinite;
#endif
    return isfinite (v.x) && isfinite (v.y) && isfinite (v.z);
}

CUDA_COMMON_FUNCTION CUDA_INLINE float4 make_float4 (float v)
{
    return make_float4 (v, v, v, v);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 make_float4 (const float3& v)
{
    return make_float4 (v.x, v.y, v.z, 0.0f);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 make_float4 (const float3& v, float w)
{
    return make_float4 (v.x, v.y, v.z, w);
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator== (const float4& v0, const float4& v1)
{
    return v0.x == v1.x && v0.y == v1.y && v0.z == v1.z && v0.w == v1.w;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool operator!= (const float4& v0, const float4& v1)
{
    return v0.x != v1.x || v0.y != v1.y || v0.z != v1.z || v0.w != v1.w;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 operator- (const float4& v)
{
    return make_float4 (-v.x, -v.y, -v.z, -v.w);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 operator+ (const float4& v0, const float4& v1)
{
    return make_float4 (v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4& operator+= (float4& v0, const float4& v1)
{
    v0.x += v1.x;
    v0.y += v1.y;
    v0.z += v1.z;
    v0.w += v1.w;
    return v0;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 operator- (const float4& v0, const float4& v1)
{
    return make_float4 (v0.x - v1.x, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4& operator-= (float4& v0, const float4& v1)
{
    v0.x -= v1.x;
    v0.y -= v1.y;
    v0.z -= v1.z;
    v0.w -= v1.w;
    return v0;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 operator* (const float4& v0, const float4& v1)
{
    return make_float4 (v0.x * v1.x, v0.y * v1.y, v0.z * v1.z, v0.w * v1.w);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 operator* (float s, const float4& v)
{
    return make_float4 (s * v.x, s * v.y, s * v.z, s * v.w);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 operator* (const float4& v, float s)
{
    return make_float4 (s * v.x, s * v.y, s * v.z, s * v.w);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4& operator*= (float4& v0, const float4& v1)
{
    v0.x *= v1.x;
    v0.y *= v1.y;
    v0.z *= v1.z;
    v0.w *= v1.w;
    return v0;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4& operator*= (float4& v, float s)
{
    v.x *= s;
    v.y *= s;
    v.z *= s;
    v.w *= s;
    return v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 operator/ (const float4& v0, const float4& v1)
{
    return make_float4 (v0.x / v1.x, v0.y / v1.y, v0.z / v1.z, v0.w / v1.w);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 operator/ (const float4& v, float s)
{
    float r = 1 / s;
    return r * v;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4& operator/= (float4& v, float s)
{
    float r = 1 / s;
    return v *= r;
}
CUDA_COMMON_FUNCTION CUDA_INLINE bool allFinite (const float4& v)
{
#if !defined(__CUDA_ARCH__)
    using std::isfinite;
#endif
    return isfinite (v.x) && isfinite (v.y) && isfinite (v.z) && isfinite (v.w);
}

CUDA_COMMON_FUNCTION CUDA_INLINE int2 min (const int2& v0, const int2& v1)
{
    return make_int2 (shared::min (v0.x, v1.x),
                      shared::min (v0.y, v1.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE int2 max (const int2& v0, const int2& v1)
{
    return make_int2 (shared::max (v0.x, v1.x),
                      shared::max (v0.y, v1.y));
}

CUDA_COMMON_FUNCTION CUDA_INLINE uint2 min (const uint2& v0, const uint2& v1)
{
    return make_uint2 (shared::min (v0.x, v1.x),
                       shared::min (v0.y, v1.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE uint2 max (const uint2& v0, const uint2& v1)
{
    return make_uint2 (shared::max (v0.x, v1.x),
                       shared::max (v0.y, v1.y));
}

CUDA_COMMON_FUNCTION CUDA_INLINE float2 min (const float2& v0, const float2& v1)
{
    return make_float2 (shared::min (v0.x, v1.x),
                        shared::min (v0.y, v1.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE float2 max (const float2& v0, const float2& v1)
{
    return make_float2 (shared::max (v0.x, v1.x),
                        shared::max (v0.y, v1.y));
}
CUDA_COMMON_FUNCTION CUDA_INLINE float cross (const float2& v0, const float2& v1)
{
    return v0.x * v1.y - v0.y * v1.x;
}

CUDA_COMMON_FUNCTION CUDA_INLINE float3 min (const float3& v0, const float3& v1)
{
    return make_float3 (std::fmin (v0.x, v1.x),
                        std::fmin (v0.y, v1.y),
                        std::fmin (v0.z, v1.z));
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 max (const float3& v0, const float3& v1)
{
    return make_float3 (std::fmax (v0.x, v1.x),
                        std::fmax (v0.y, v1.y),
                        std::fmax (v0.z, v1.z));
}
CUDA_COMMON_FUNCTION CUDA_INLINE float dot (const float3& v0, const float3& v1)
{
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 cross (const float3& v0, const float3& v1)
{
    return make_float3 (v0.y * v1.z - v0.z * v1.y,
                        v0.z * v1.x - v0.x * v1.z,
                        v0.x * v1.y - v0.y * v1.x);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float squaredDistance (const float3& p0, const float3& p1)
{
    float3 d = p1 - p0;
    return dot (d, d);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float length (const float3& v)
{
    return std::sqrt (v.x * v.x + v.y * v.y + v.z * v.z);
}
CUDA_COMMON_FUNCTION CUDA_INLINE float sqLength (const float3& v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
CUDA_COMMON_FUNCTION CUDA_INLINE float3 normalize (const float3& v)
{
    return v / length (v);
}

CUDA_COMMON_FUNCTION CUDA_INLINE float4 min (const float4& v0, const float4& v1)
{
    return make_float4 (std::fmin (v0.x, v1.x),
                        std::fmin (v0.y, v1.y),
                        std::fmin (v0.z, v1.z),
                        std::fmin (v0.w, v1.w));
}
CUDA_COMMON_FUNCTION CUDA_INLINE float4 max (const float4& v0, const float4& v1)
{
    return make_float4 (std::fmax (v0.x, v1.x),
                        std::fmax (v0.y, v1.y),
                        std::fmax (v0.z, v1.z),
                        std::fmax (v0.w, v1.w));
}
CUDA_COMMON_FUNCTION CUDA_INLINE float dot (const float4& v0, const float4& v1)
{
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
}

// ( 0, 0,  1) <=> phi:      0
// (-1, 0,  0) <=> phi: 1/2 pi
// ( 0, 0, -1) <=> phi:   1 pi
// ( 1, 0,  0) <=> phi: 3/2 pi
CUDA_DEVICE_FUNCTION CUDA_INLINE float3 fromPolarYUp (float phi, float theta)
{
    float sinPhi, cosPhi;
    float sinTheta, cosTheta;
#if defined(__CUDA_ARCH__)
    sincosf (phi, &sinPhi, &cosPhi);
    sincosf (theta, &sinTheta, &cosTheta);
#else
    sinPhi = std::sin (phi);
    cosPhi = std::cos (phi);
    sinTheta = std::sin (theta);
    cosTheta = std::cos (theta);
#endif
    return make_float3 (-sinPhi * sinTheta, cosTheta, cosPhi * sinTheta);
}

CUDA_DEVICE_FUNCTION CUDA_INLINE float3 HSVtoRGB (float h, float s, float v)
{
    if (s == 0)
        return make_float3 (v, v, v);

    h = h - std::floor (h);
    int32_t hi = static_cast<int32_t> (h * 6);
    float f = h * 6 - hi;
    float m = v * (1 - s);
    float n = v * (1 - s * f);
    float k = v * (1 - s * (1 - f));
    if (hi == 0)
        return make_float3 (v, k, m);
    else if (hi == 1)
        return make_float3 (n, v, m);
    else if (hi == 2)
        return make_float3 (m, v, k);
    else if (hi == 3)
        return make_float3 (m, n, v);
    else if (hi == 4)
        return make_float3 (k, m, v);
    else if (hi == 5)
        return make_float3 (v, m, n);
    return make_float3 (0, 0, 0);
}

CUDA_DEVICE_FUNCTION CUDA_INLINE float3 calcFalseColor (float value, float minValue, float maxValue)
{
    float t = (value - minValue) / (maxValue - minValue);
    t = fmin (fmax (t, 0.0f), 1.0f);
    constexpr float3 R = {1.0f, 0.0f, 0.0f};
    constexpr float3 G = {0.0f, 1.0f, 0.0f};
    constexpr float3 B = {0.0f, 0.0f, 1.0f};
    float3 ret;
    if (t < 0.5f)
    {
        t = (t - 0.0f) / 0.5f;
        ret = B * (1 - t) + G * t;
    }
    else
    {
        t = (t - 0.5f) / 0.5f;
        ret = G * (1 - t) + R * t;
    }
    return ret;
}

CUDA_DEVICE_FUNCTION CUDA_INLINE float simpleToneMap_s (float value)
{
    Assert (value >= 0, "Input value must be equal to or greater than 0: %g", value);
    return 1 - std::exp (-value);
}

CUDA_DEVICE_FUNCTION CUDA_INLINE float sRGB_degamma_s (float value)
{
    Assert (value >= 0, "Input value must be equal to or greater than 0: %g", value);
    if (value <= 0.04045f)
        return value / 12.92f;
    return std::pow ((value + 0.055f) / 1.055f, 2.4f);
}

CUDA_DEVICE_FUNCTION CUDA_INLINE float sRGB_gamma_s (float value)
{
    Assert (value >= 0, "Input value must be equal to or greater than 0: %g", value);
    if (value <= 0.0031308f)
        return 12.92f * value;
    return 1.055f * std::pow (value, 1 / 2.4f) - 0.055f;
}

CUDA_DEVICE_FUNCTION CUDA_INLINE float3 sRGB_degamma (const float3& value)
{
    return make_float3 (sRGB_degamma_s (value.x),
                        sRGB_degamma_s (value.y),
                        sRGB_degamma_s (value.z));
}

CUDA_COMMON_FUNCTION CUDA_INLINE int32_t floatToOrderedInt (float fVal)
{
#if defined(__CUDA_ARCH__)
    int32_t iVal = __float_as_int (fVal);
#else
    int32_t iVal = *reinterpret_cast<int32_t*> (&fVal);
#endif
    return (iVal >= 0) ? iVal : iVal ^ 0x7FFFFFFF;
}

CUDA_COMMON_FUNCTION CUDA_INLINE float orderedIntToFloat (int32_t iVal)
{
    int32_t orgVal = (iVal >= 0) ? iVal : iVal ^ 0x7FFFFFFF;
#if defined(__CUDA_ARCH__)
    return __int_as_float (orgVal);
#else
    return *reinterpret_cast<float*> (&orgVal);
#endif
}

struct float3AsOrderedInt
{
    int32_t x, y, z;

    CUDA_COMMON_FUNCTION float3AsOrderedInt() :
        x (0), y (0), z (0)
    {
    }
    CUDA_COMMON_FUNCTION float3AsOrderedInt (const float3& v) :
        x (floatToOrderedInt (v.x)), y (floatToOrderedInt (v.y)), z (floatToOrderedInt (v.z))
    {
    }

    CUDA_COMMON_FUNCTION explicit operator float3() const
    {
        return make_float3 (orderedIntToFloat (x), orderedIntToFloat (y), orderedIntToFloat (z));
    }
};

struct AABB
{
    float3 minP;
    float3 maxP;

    CUDA_COMMON_FUNCTION AABB() :
        minP (make_float3 (INFINITY)), maxP (make_float3 (-INFINITY)) {}
    CUDA_COMMON_FUNCTION AABB (const float3& _minP, const float3& _maxP) :
        minP (_minP), maxP (_maxP) {}

    CUDA_COMMON_FUNCTION AABB& unify (const float3& p)
    {
        minP = min (minP, p);
        maxP = max (maxP, p);
        return *this;
    }

    CUDA_COMMON_FUNCTION AABB& dilate (float scale)
    {
        float3 d = maxP - minP;
        minP -= 0.5f * (scale - 1) * d;
        maxP += 0.5f * (scale - 1) * d;
        return *this;
    }

    CUDA_COMMON_FUNCTION float calcHalfSurfaceArea() const
    {
        float3 d = maxP - minP;
        return d.x * d.y + d.y * d.z + d.z * d.x;
    }
};

struct AABBAsOrderedInt
{
    float3AsOrderedInt minP;
    float3AsOrderedInt maxP;

    CUDA_COMMON_FUNCTION AABBAsOrderedInt() :
        minP (make_float3 (INFINITY)), maxP (make_float3 (-INFINITY))
    {
    }
    CUDA_COMMON_FUNCTION AABBAsOrderedInt (const AABB& v) :
        minP (v.minP), maxP (v.maxP)
    {
    }

    CUDA_COMMON_FUNCTION AABBAsOrderedInt& operator= (const AABBAsOrderedInt& v)
    {
        minP = v.minP;
        maxP = v.maxP;
        return *this;
    }

    CUDA_COMMON_FUNCTION explicit operator AABB() const
    {
        return AABB (static_cast<float3> (minP), static_cast<float3> (maxP));
    }
};

struct Matrix3x3
{
    union
    {
        struct
        {
            float m00, m10, m20;
        };
        float3 c0;
    };
    union
    {
        struct
        {
            float m01, m11, m21;
        };
        float3 c1;
    };
    union
    {
        struct
        {
            float m02, m12, m22;
        };
        float3 c2;
    };

    CUDA_COMMON_FUNCTION /*constexpr*/ Matrix3x3() :
        c0 (make_float3 (1, 0, 0)),
        c1 (make_float3 (0, 1, 0)),
        c2 (make_float3 (0, 0, 1)) {}
    CUDA_COMMON_FUNCTION Matrix3x3 (const float array[9]) :
        m00 (array[0]), m10 (array[1]), m20 (array[2]),
        m01 (array[3]), m11 (array[4]), m21 (array[5]),
        m02 (array[6]), m12 (array[7]), m22 (array[8]) {}
    CUDA_COMMON_FUNCTION /*constexpr*/ Matrix3x3 (const float3& col0, const float3& col1, const float3& col2) :
        c0 (col0), c1 (col1), c2 (col2)
    {
    }

    CUDA_COMMON_FUNCTION Matrix3x3 operator+() const { return *this; }
    CUDA_COMMON_FUNCTION Matrix3x3 operator-() const { return Matrix3x3 (-c0, -c1, -c2); }

    CUDA_COMMON_FUNCTION Matrix3x3& operator+= (const Matrix3x3& mat)
    {
        c0 += mat.c0;
        c1 += mat.c1;
        c2 += mat.c2;
        return *this;
    }
    CUDA_COMMON_FUNCTION Matrix3x3& operator-= (const Matrix3x3& mat)
    {
        c0 -= mat.c0;
        c1 -= mat.c1;
        c2 -= mat.c2;
        return *this;
    }
    CUDA_COMMON_FUNCTION Matrix3x3& operator*= (float s)
    {
        c0 *= s;
        c1 *= s;
        c2 *= s;
        return *this;
    }
    CUDA_COMMON_FUNCTION Matrix3x3& operator*= (const Matrix3x3& mat)
    {
        const float3 r[] = {row (0), row (1), row (2)};
        c0 = make_float3 (dot (r[0], mat.c0), dot (r[1], mat.c0), dot (r[2], mat.c0));
        c1 = make_float3 (dot (r[0], mat.c1), dot (r[1], mat.c1), dot (r[2], mat.c1));
        c2 = make_float3 (dot (r[0], mat.c2), dot (r[1], mat.c2), dot (r[2], mat.c2));
        return *this;
    }

    CUDA_COMMON_FUNCTION float3 operator* (const float3& v) const
    {
        const float3 r[] = {row (0), row (1), row (2)};
        return make_float3 (dot (r[0], v),
                            dot (r[1], v),
                            dot (r[2], v));
    }

    CUDA_COMMON_FUNCTION float3 row (unsigned int r) const
    {
        // Assert(r < 3, "\"r\" is out of range [0, 2].");
        switch (r)
        {
            case 0:
                return make_float3 (m00, m01, m02);
            case 1:
                return make_float3 (m10, m11, m12);
            case 2:
                return make_float3 (m20, m21, m22);
            default:
                return make_float3 (0, 0, 0);
        }
    }

    CUDA_COMMON_FUNCTION Matrix3x3& inverse()
    {
        float det = 1.0f / (m00 * m11 * m22 + m01 * m12 * m20 + m02 * m10 * m21 -
                            m02 * m11 * m20 - m01 * m10 * m22 - m00 * m12 * m21);
        Matrix3x3 m;
        m.m00 = det * (m11 * m22 - m12 * m21);
        m.m01 = -det * (m01 * m22 - m02 * m21);
        m.m02 = det * (m01 * m12 - m02 * m11);
        m.m10 = -det * (m10 * m22 - m12 * m20);
        m.m11 = det * (m00 * m22 - m02 * m20);
        m.m12 = -det * (m00 * m12 - m02 * m10);
        m.m20 = det * (m10 * m21 - m11 * m20);
        m.m21 = -det * (m00 * m21 - m01 * m20);
        m.m22 = det * (m00 * m11 - m01 * m10);
        *this = m;

        return *this;
    }
    CUDA_COMMON_FUNCTION Matrix3x3& transpose()
    {
        float temp;
        temp = m10;
        m10 = m01;
        m01 = temp;
        temp = m20;
        m20 = m02;
        m02 = temp;
        temp = m21;
        m21 = m12;
        m12 = temp;
        return *this;
    }
};

CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 operator+ (const Matrix3x3& a, const Matrix3x3& b)
{
    Matrix3x3 ret = a;
    ret += b;
    return ret;
}
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 operator- (const Matrix3x3& a, const Matrix3x3& b)
{
    Matrix3x3 ret = a;
    ret -= b;
    return ret;
}
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 operator* (const Matrix3x3& a, float b)
{
    Matrix3x3 ret = a;
    ret *= b;
    return ret;
}
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 operator* (float a, const Matrix3x3& b)
{
    Matrix3x3 ret = b;
    ret *= a;
    return ret;
}
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 operator* (const Matrix3x3& a, const Matrix3x3& b)
{
    Matrix3x3 ret = a;
    ret *= b;
    return ret;
}

CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 transpose (const Matrix3x3& mat)
{
    Matrix3x3 ret = mat;
    return ret.transpose();
}
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 inverse (const Matrix3x3& mat)
{
    Matrix3x3 ret = mat;
    return ret.inverse();
}

CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 scale3x3 (const float3& s)
{
    return Matrix3x3 (s.x * make_float3 (1, 0, 0),
                      s.y * make_float3 (0, 1, 0),
                      s.z * make_float3 (0, 0, 1));
}
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 scale3x3 (float sx, float sy, float sz)
{
    return scale3x3 (make_float3 (sx, sy, sz));
}
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 scale3x3 (float s)
{
    return scale3x3 (make_float3 (s, s, s));
}

CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 rotate3x3 (float angle, const float3& axis)
{
    Matrix3x3 matrix;
    float3 nAxis = normalize (axis);
    float s = std::sin (angle);
    float c = std::cos (angle);
    float oneMinusC = 1 - c;

    matrix.m00 = nAxis.x * nAxis.x * oneMinusC + c;
    matrix.m10 = nAxis.x * nAxis.y * oneMinusC + nAxis.z * s;
    matrix.m20 = nAxis.z * nAxis.x * oneMinusC - nAxis.y * s;
    matrix.m01 = nAxis.x * nAxis.y * oneMinusC - nAxis.z * s;
    matrix.m11 = nAxis.y * nAxis.y * oneMinusC + c;
    matrix.m21 = nAxis.y * nAxis.z * oneMinusC + nAxis.x * s;
    matrix.m02 = nAxis.z * nAxis.x * oneMinusC + nAxis.y * s;
    matrix.m12 = nAxis.y * nAxis.z * oneMinusC - nAxis.x * s;
    matrix.m22 = nAxis.z * nAxis.z * oneMinusC + c;

    return matrix;
}
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 rotate3x3 (float angle, float ax, float ay, float az)
{
    return rotate3x3 (angle, make_float3 (ax, ay, az));
}
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 rotateX3x3 (float angle) { return rotate3x3 (angle, make_float3 (1, 0, 0)); }
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 rotateY3x3 (float angle) { return rotate3x3 (angle, make_float3 (0, 1, 0)); }
CUDA_COMMON_FUNCTION CUDA_INLINE Matrix3x3 rotateZ3x3 (float angle) { return rotate3x3 (angle, make_float3 (0, 0, 1)); }

struct Quaternion
{
    union
    {
        float3 v;
        struct
        {
            float x;
            float y;
            float z;
        };
    };
    float w;

    CUDA_COMMON_FUNCTION constexpr Quaternion() :
        v(), w (1) {}
    CUDA_COMMON_FUNCTION /*constexpr*/ Quaternion (float xx, float yy, float zz, float ww) :
        v (make_float3 (xx, yy, zz)), w (ww) {}
    CUDA_COMMON_FUNCTION constexpr Quaternion (const float3& vv, float ww) :
        v (vv), w (ww) {}

    CUDA_COMMON_FUNCTION Quaternion operator+() const { return *this; }
    CUDA_COMMON_FUNCTION Quaternion operator-() const { return Quaternion (-v, -w); }

    CUDA_COMMON_FUNCTION Quaternion& operator*= (float s)
    {
        v *= s;
        w *= s;
        return *this;
    }
    CUDA_COMMON_FUNCTION Quaternion& operator*= (const Quaternion& r)
    {
        const float3 v_ = cross (v, r.v) + w * r.v + r.w * v;
        w = w * r.w - dot (v, r.v);
        v = v_;
        return *this;
    }
    CUDA_COMMON_FUNCTION Quaternion& operator/= (float s)
    {
        const float r = 1 / s;
        return *this *= r;
    }

    CUDA_COMMON_FUNCTION void toEulerAngles (float* roll, float* pitch, float* yaw) const
    {
        float xx = x * x;
        float xy = x * y;
        float xz = x * z;
        float xw = x * w;
        float yy = y * y;
        float yz = y * z;
        float yw = y * w;
        float zz = z * z;
        float zw = z * w;
        float ww = w * w;
        *pitch = std::atan2 (2 * (xw + yz), ww - xx - yy + zz);                   // around x
        *yaw = std::asin (std::fmin (std::fmax (2.0f * (yw - xz), -1.0f), 1.0f)); // around y
        *roll = std::atan2 (2 * (zw + xy), ww + xx - yy - zz);                    // around z
    }
    CUDA_COMMON_FUNCTION Matrix3x3 toMatrix3x3() const
    {
        float xx = x * x, yy = y * y, zz = z * z;
        float xy = x * y, yz = y * z, zx = z * x;
        float xw = x * w, yw = y * w, zw = z * w;
        return Matrix3x3 (make_float3 (1 - 2 * (yy + zz), 2 * (xy + zw), 2 * (zx - yw)),
                          make_float3 (2 * (xy - zw), 1 - 2 * (xx + zz), 2 * (yz + xw)),
                          make_float3 (2 * (zx + yw), 2 * (yz - xw), 1 - 2 * (xx + yy)));
    }
};

CUDA_COMMON_FUNCTION CUDA_INLINE Quaternion operator* (const Quaternion& a, float b)
{
    Quaternion ret = a;
    ret *= b;
    return ret;
}
CUDA_COMMON_FUNCTION CUDA_INLINE Quaternion operator* (const Quaternion& a, const Quaternion& b)
{
    Quaternion ret = a;
    ret *= b;
    return ret;
}
CUDA_COMMON_FUNCTION CUDA_INLINE Quaternion operator/ (const Quaternion& a, float b)
{
    Quaternion ret = a;
    ret /= b;
    return ret;
}
CUDA_COMMON_FUNCTION CUDA_INLINE Quaternion operator* (float a, const Quaternion& b)
{
    Quaternion ret = b;
    ret *= a;
    return ret;
}

CUDA_COMMON_FUNCTION CUDA_INLINE static Quaternion qRotate (float angle, const float3& axis)
{
    float ha = angle / 2;
    float s = std::sin (ha), c = std::cos (ha);
    return Quaternion (s * normalize (axis), c);
}
CUDA_COMMON_FUNCTION CUDA_INLINE static Quaternion qRotate (float angle, float ax, float ay, float az)
{
    return qRotate (angle, make_float3 (ax, ay, az));
}
CUDA_COMMON_FUNCTION CUDA_INLINE static Quaternion qRotateX (float angle) { return qRotate (angle, make_float3 (1, 0, 0)); }
CUDA_COMMON_FUNCTION CUDA_INLINE static Quaternion qRotateY (float angle) { return qRotate (angle, make_float3 (0, 1, 0)); }
CUDA_COMMON_FUNCTION CUDA_INLINE static Quaternion qRotateZ (float angle) { return qRotate (angle, make_float3 (0, 0, 1)); }

CUDA_COMMON_FUNCTION CUDA_INLINE Quaternion qFromEulerAngles (float roll, float pitch, float yaw)
{
    return qRotateZ (roll) * qRotateY (yaw) * qRotateX (pitch);
}

// EN: Definitions only for host.
#if !defined(__CUDACC__) || defined(OPTIXU_Platform_CodeCompletion)

#if 1
#define hpprintf(fmt, ...) \
    do \
    { \
        devPrintf (fmt, ##__VA_ARGS__); \
        printf (fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define hpprintf(fmt, ...) printf (fmt, ##__VA_ARGS__)
#endif

#endif