#pragma once

// taken from Shocker GfxExp
// https://github.com/shocker-0x15/GfxExp


#include <mace_core/mace_core.h>
#include "common_shared.h"

template <std::floating_point T>
static constexpr T pi_v = std::numbers::pi_v<T>;

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

template <typename T, typename Deleter, typename... ArgTypes>
std::shared_ptr<T> make_shared_with_deleter (const Deleter& deleter, ArgTypes&&... args)
{
    return std::shared_ptr<T> (new T (std::forward<ArgTypes> (args)...),
                               deleter);
}

template <uint32_t numBuffers>
class StreamChain
{
    std::array<CUstream, numBuffers> m_streams;
    std::array<CUevent, numBuffers> m_endEvents;
    uint32_t m_curBufIdx;

 public:
    StreamChain()
    {
        for (int i = 0; i < numBuffers; ++i)
        {
            m_streams[i] = nullptr;
            m_endEvents[i] = nullptr;
        }
    }

    void initialize (CUcontext cuContext)
    {
        for (int i = 0; i < numBuffers; ++i)
        {
            CUDADRV_CHECK (cuStreamCreate (&m_streams[i], 0));
            CUDADRV_CHECK (cuEventCreate (&m_endEvents[i], 0));
        }
        m_curBufIdx = 0;
    }

    void finalize()
    {
        for (int i = 1; i >= 0; --i)
        {
            CUDADRV_CHECK (cuStreamSynchronize (m_streams[i]));
            CUDADRV_CHECK (cuEventDestroy (m_endEvents[i]));
            CUDADRV_CHECK (cuStreamDestroy (m_streams[i]));
            m_endEvents[i] = nullptr;
            m_streams[i] = nullptr;
        }
    }

    void swap()
    {
        CUstream curStream = m_streams[m_curBufIdx];
        CUevent curEvent = m_endEvents[m_curBufIdx];
        CUDADRV_CHECK (cuEventRecord (curEvent, curStream));
        m_curBufIdx = (m_curBufIdx + 1) % numBuffers;
    }

    CUstream waitAvailableAndGetCurrentStream() const
    {
        CUstream curStream = m_streams[m_curBufIdx];
        CUevent prevStreamEndEvent = m_endEvents[(m_curBufIdx + numBuffers - 1) % numBuffers];
        CUDADRV_CHECK (cuStreamSynchronize (curStream));
        CUDADRV_CHECK (cuStreamWaitEvent (curStream, prevStreamEndEvent, 0));
        return curStream;
    }

    void waitAllWorkDone() const
    {
        for (int i = 0; i < numBuffers; ++i)
            CUDADRV_CHECK (cuStreamSynchronize (m_streams[i]));
    }
};

template <typename RealType>
class DiscreteDistribution1DTemplate
{
    cudau::TypedBuffer<RealType> m_weights;
    cudau::TypedBuffer<RealType> m_CDF;
    RealType m_integral;
    uint32_t m_numValues;
    unsigned int m_isInitialized : 1;

 public:
    DiscreteDistribution1DTemplate() :
        m_integral (0.0f), m_numValues (0), m_isInitialized (false) {}
    void initialize (
        CUcontext cuContext, cudau::BufferType type,
        const RealType* values, uint32_t numValues);
    void finalize()
    {
        if (!m_isInitialized)
            return;

        if (m_CDF.isInitialized() && m_weights.isInitialized())
        {
            m_CDF.finalize();
            m_weights.finalize();
        }
    }

    DiscreteDistribution1DTemplate& operator= (DiscreteDistribution1DTemplate&& v)
    {
        m_weights = std::move (v.m_weights);
        m_integral = v.m_integral;
        m_numValues = v.m_numValues;
        return *this;
    }

    RealType getIntengral() const
    {
        return m_integral;
    }

    bool isInitialized() const { return m_isInitialized; }

    void getDeviceType (shared::DiscreteDistribution1DTemplate<RealType>* instance) const
    {
        new (instance) shared::DiscreteDistribution1DTemplate<RealType> (
            m_weights.isInitialized() ? m_weights.getDevicePointer() : nullptr,
            m_CDF.isInitialized() ? m_CDF.getDevicePointer() : nullptr,
            m_integral, m_numValues);
    }

    RealType* weightsOnDevice() const
    {
        return m_weights.getDevicePointer();
    }

#if !defined(USE_WALKER_ALIAS_METHOD)
    RealType* cdfOnDevice() const
    {
        return m_CDF.getDevicePointer();
    }
#endif
};

template <typename RealType>
class RegularConstantContinuousDistribution1DTemplate
{
    cudau::TypedBuffer<RealType> m_PDF;
    cudau::TypedBuffer<RealType> m_CDF;

    RealType m_integral;
    uint32_t m_numValues;
    unsigned int m_isInitialized : 1;

 public:
    RegularConstantContinuousDistribution1DTemplate() :
        m_isInitialized (false) {}

    void initialize (
        CUcontext cuContext, cudau::BufferType type,
        const RealType* values, uint32_t numValues);
    void finalize (CUcontext cuContext)
    {
        if (!m_isInitialized)
            return;
        if (m_CDF.isInitialized() && m_PDF.isInitialized())
        {
            m_CDF.finalize();
            m_PDF.finalize();
        }
    }

    RegularConstantContinuousDistribution1DTemplate& operator= (RegularConstantContinuousDistribution1DTemplate&& v)
    {
        m_PDF = std::move (v.m_PDF);
        m_CDF = std::move (v.m_CDF);
        m_integral = v.m_integral;
        m_numValues = v.m_numValues;
        return *this;
    }

    RealType getIntegral() const { return m_integral; }
    uint32_t getNumValues() const { return m_numValues; }

    bool isInitialized() const { return m_isInitialized; }

    void getDeviceType (shared::RegularConstantContinuousDistribution1DTemplate<RealType>* instance) const
    {
        new (instance) shared::RegularConstantContinuousDistribution1DTemplate<RealType> (
            m_PDF.getDevicePointer(), m_CDF.getDevicePointer(), m_integral, m_numValues);
    }
};

template <typename RealType>
class RegularConstantContinuousDistribution2DTemplate
{
    cudau::TypedBuffer<shared::RegularConstantContinuousDistribution1DTemplate<RealType>> m_raw1DDists;
    RegularConstantContinuousDistribution1DTemplate<RealType>* m_1DDists;
    RegularConstantContinuousDistribution1DTemplate<RealType> m_top1DDist;
    unsigned int m_isInitialized : 1;

 public:
    RegularConstantContinuousDistribution2DTemplate() :
        m_1DDists (nullptr), m_isInitialized (false) {}

    RegularConstantContinuousDistribution2DTemplate& operator= (RegularConstantContinuousDistribution2DTemplate&& v)
    {
        m_raw1DDists = std::move (v.m_raw1DDists);
        m_1DDists = std::move (v.m_1DDists);
        m_top1DDist = std::move (v.m_top1DDist);
        return *this;
    }

    void initialize (
        CUcontext cuContext, cudau::BufferType type,
        const RealType* values, uint32_t numD1, uint32_t numD2);
    void finalize (CUcontext cuContext)
    {
        if (!m_isInitialized)
            return;

        m_top1DDist.finalize (cuContext);

        for (int i = m_top1DDist.getNumValues() - 1; i >= 0; --i)
        {
            m_1DDists[i].finalize (cuContext);
        }

        m_raw1DDists.finalize();
        delete[] m_1DDists;
        m_1DDists = nullptr;
    }

    bool isInitialized() const { return m_isInitialized; }

    void getDeviceType (shared::RegularConstantContinuousDistribution2DTemplate<RealType>* instance) const
    {
        shared::RegularConstantContinuousDistribution1DTemplate<RealType> top1DDist;
        m_top1DDist.getDeviceType (&top1DDist);
        new (instance) shared::RegularConstantContinuousDistribution2DTemplate<RealType> (
            m_raw1DDists.getDevicePointer(), top1DDist);
    }
};

using DiscreteDistribution1D = DiscreteDistribution1DTemplate<float>;
using RegularConstantContinuousDistribution1D = RegularConstantContinuousDistribution1DTemplate<float>;
using RegularConstantContinuousDistribution2D = RegularConstantContinuousDistribution2DTemplate<float>;
