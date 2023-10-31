// // taken from OptiX_Utility
// https://github.com/shocker-0x15/OptiX_Utility/blob/master/LICENSE.md

#include "common_host.h"

void devPrintf (const char* fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    char str[4096];
    vsnprintf_s (str, sizeof (str), _TRUNCATE, fmt, args);
    va_end (args);
    OutputDebugString (str);
}

// The initialize method implementation
template <typename RealType>
void DiscreteDistribution1DTemplate<RealType>::
    initialize (
        CUcontext cuContext, cudau::BufferType type,
        const RealType* values, uint32_t numValues)
{
    // Assert that we're not already initialized
    Assert (!m_isInitialized, "Already initialized!");

    // Set the number of values
    m_numValues = numValues;

    // Handle empty input
    if (m_numValues == 0)
    {
        m_integral = 0.0f;
        return;
    }

    // Initialize weight and CDF buffers
    m_weights.initialize (cuContext, type, m_numValues);
    m_CDF.initialize (cuContext, type, m_numValues);

    // If input values are null, set integral to zero and return
    if (values == nullptr)
    {
        m_integral = 0.0f;
        m_isInitialized = true;
        return;
    }

    // Copy values into weight buffer
    RealType* weights = m_weights.map();
    std::memcpy (weights, values, sizeof (RealType) * m_numValues);
    m_weights.unmap();

    // Compute the CDF
    RealType* CDF = m_CDF.map();
    CompensatedSum_T<RealType> sum (0);
    for (uint32_t i = 0; i < m_numValues; ++i)
    {
        CDF[i] = sum;
        sum += values[i];
    }
    m_integral = sum;

    // Unmap CDF buffer and set initialized flag
    m_CDF.unmap();
    m_isInitialized = true;
}

template class DiscreteDistribution1DTemplate<float>;

template <typename RealType>
void RegularConstantContinuousDistribution1DTemplate<RealType>::
    initialize (
        CUcontext cuContext, cudau::BufferType type,
        const RealType* values, uint32_t numValues)
{
    Assert (!m_isInitialized, "Already initialized!");
    m_numValues = numValues;

    m_PDF.initialize (cuContext, type, m_numValues);
    m_CDF.initialize (cuContext, type, m_numValues + 1);

    RealType* PDF = m_PDF.map();
    RealType* CDF = m_CDF.map();
    std::memcpy (PDF, values, sizeof (RealType) * m_numValues);

    CompensatedSum_T<RealType> sum{0};
    for (uint32_t i = 0; i < m_numValues; ++i)
    {
        CDF[i] = sum;
        sum += PDF[i] / m_numValues;
    }
    m_integral = sum;
    for (uint32_t i = 0; i < m_numValues; ++i)
    {
        PDF[i] /= m_integral;
        CDF[i] /= m_integral;
    }
    CDF[m_numValues] = 1.0f;

    m_CDF.unmap();
    m_PDF.unmap();

    m_isInitialized = true;
}

template class RegularConstantContinuousDistribution1DTemplate<float>;

template <typename RealType>
void RegularConstantContinuousDistribution2DTemplate<RealType>::
    initialize (
        CUcontext cuContext, cudau::BufferType type,
        const RealType* values, uint32_t numD1, uint32_t numD2)
{
    Assert (!m_isInitialized, "Already initialized!");
    m_1DDists = new RegularConstantContinuousDistribution1DTemplate<RealType>[numD2];
    m_raw1DDists.initialize (cuContext, type, static_cast<uint32_t> (numD2));

    shared::RegularConstantContinuousDistribution1DTemplate<RealType>* rawDists = m_raw1DDists.map();

    // EN: First, create Distribution1D's for every rows.
    CompensatedSum_T<RealType> sum (0);
    RealType* integrals = new RealType[numD2];
    for (uint32_t i = 0; i < numD2; ++i)
    {
        RegularConstantContinuousDistribution1DTemplate<RealType>& dist = m_1DDists[i];
        dist.initialize (cuContext, type, values + i * numD1, numD1);
        dist.getDeviceType (&rawDists[i]);
        integrals[i] = dist.getIntegral();
        sum += integrals[i];
    }

    // EN: create a Distribution1D using integral values of each row.
    m_top1DDist.initialize (cuContext, type, integrals, numD2);
    delete[] integrals;

    Assert (std::isfinite (m_top1DDist.getIntegral()), "invalid integral value.");

    m_raw1DDists.unmap();

    m_isInitialized = true;
}

template class RegularConstantContinuousDistribution2DTemplate<float>;
