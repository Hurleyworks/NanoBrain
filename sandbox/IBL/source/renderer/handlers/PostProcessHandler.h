/*
MIT License

Copyright (c) 2023 Steve Hurley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

// much taken from OptiX_Utility
// https://github.com/shocker-0x15/OptiX_Utility/blob/master/LICENSE.md

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
