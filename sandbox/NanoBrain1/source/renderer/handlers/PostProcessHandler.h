
#pragma once

#pragma once

#include "../RenderContext.h"

using PostProcessHandlerRef = std::shared_ptr<class PostProcessHandler>;

class PostProcessHandler
{
 public:
    static PostProcessHandlerRef create (RenderContextPtr ctx) { return std::make_shared<PostProcessHandler> (ctx); }

 public:
    PostProcessHandler (RenderContextPtr ctx);
    ~PostProcessHandler();

    void initialize();
    void resize (uint32_t width, uint32_t height);
    void finalize();
    void tonemap();

    void denoise (bool newSequence);
    void getRender (BufferToDisplay bufferTypeToDisplay);
    void saveRender (uint32_t frameNumber);

    CUsurfObject getBeautyBuffer() { return beautyAccumBuffer.getSurfaceObject (0); }
    CUsurfObject getNormalBuffer() { return normalAccumBuffer.getSurfaceObject (0); }
    CUsurfObject getAlbedoBuffer() { return albedoAccumBuffer.getSurfaceObject (0); }

 private:
    RenderContextPtr ctx = nullptr;

    uint32_t tileWidth = 0;
    uint32_t tileHeight = 0;
    bool useKernelPredictionMode = false;
    bool performUpscale = false;
    bool useAlbedo = true;
    bool useNormal = true;

    optixu::Denoiser denoiser;
    cudau::Buffer denoiserStateBuffer;
    cudau::Buffer denoiserScratchBuffer;

    cudau::Array beautyAccumBuffer;
    cudau::Array albedoAccumBuffer;
    cudau::Array normalAccumBuffer;

    cudau::TypedBuffer<float4> linearBeautyBuffer;
    cudau::TypedBuffer<float4> linearAlbedoBuffer;
    cudau::TypedBuffer<float4> linearNormalBuffer;
    cudau::TypedBuffer<float2> linearFlowBuffer;
    cudau::TypedBuffer<float4> linearDenoisedBeautyBuffer;

    std::vector<optixu::DenoisingTask> denoisingTasks;

    // Denoiser requires linear buffers as input/output, so we need to copy the results.
    CUmodule moduleCopyBuffers;
    cudau::Kernel kernelCopyToLinearBuffers;
    cudau::Kernel kernelVisualizeToOutputBuffer;

    CUdeviceptr hdrNormalizer;

    void initializeScreenRelatedBuffers (uint32_t width, uint32_t height);
}; // end class PostProcessHandler
