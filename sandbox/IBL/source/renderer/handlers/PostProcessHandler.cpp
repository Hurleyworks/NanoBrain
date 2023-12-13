// mostly taken from OptiX_Utility
// https://github.com/shocker-0x15/OptiX_Utility/blob/master/LICENSE.md

#include "PostProcessHandler.h"
#include "../Shared.h"
#include "Handlers.h"

// ctor
PostProcessHandler::PostProcessHandler (RenderContextPtr ctx) :
    ctx (ctx)
{
}

// dtor
PostProcessHandler::~PostProcessHandler()
{
}

void PostProcessHandler::initialize()
{
    std::filesystem::path& resourcePath = ctx->resourceFolder;
    std::filesystem::path ptxFile = resourcePath / "ptx" / "copy_buffers.ptx";

    // Read the binary PTX file
    const std::vector<char> optixIr = readBinaryFile (ptxFile);
    // = readBinaryFile (data.ptxFile);
    if (optixIr.size() == 0)
        throw std::runtime_error ("ptxFile failed to load");

    const Eigen::Vector2i& renderSize = ctx->renderSize;

    initializeScreenRelatedBuffers (renderSize.x(), renderSize.y());

    OptixDenoiserModelKind denoiserModel = OPTIX_DENOISER_MODEL_KIND_HDR;
    if (performUpscale)
        denoiserModel = OPTIX_DENOISER_MODEL_KIND_UPSCALE2X;
    // Use kernel prediction model (AOV denoiser) even if this sample doesn't give any AOV inputs.
    else if (useKernelPredictionMode)
        denoiserModel = OPTIX_DENOISER_MODEL_KIND_AOV;

   
    denoiser = ctx->optCtx.createDenoiser (
        denoiserModel, useAlbedo, useNormal, OPTIX_DENOISER_ALPHA_MODE_COPY);

    size_t scratchSizeForComputeIntensity;

    optixu::DenoiserSizes denoiserSizes;
    uint32_t numTasks;
    denoiser.prepare (
        renderSize.x(), renderSize.y(), tileWidth, tileHeight,
        &denoiserSizes, &numTasks);

    LOG (DBUG) << "Denoiser State Buffer: " << denoiserSizes.stateSize;
    LOG (DBUG) << "Denoiser Scratch Buffer: " << denoiserSizes.scratchSize;
    LOG (DBUG) << "Compute Normalizer Scratch Buffer:" << denoiserSizes.scratchSizeForComputeNormalizer;

    denoiserStateBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device, denoiserSizes.stateSize, 1);
    denoiserScratchBuffer.initialize (
        ctx->cuCtx, cudau::BufferType::Device,
        std::max (denoiserSizes.scratchSize, denoiserSizes.scratchSizeForComputeNormalizer), 1);

    denoisingTasks.resize (numTasks);
    denoiser.getTasks (denoisingTasks.data());
    denoiser.setupState (ctx->cuStr, denoiserStateBuffer, denoiserScratchBuffer);

    // Denoiser requires linear buffers as input/output, so we need to copy the results.
    CUDADRV_CHECK (cuModuleLoad (&moduleCopyBuffers, ptxFile.generic_string().c_str()));

    kernelCopyToLinearBuffers.set (moduleCopyBuffers, "copyBuffers", cudau::dim3 (8, 8), 0);

    CUDADRV_CHECK (cuMemAlloc (&hdrNormalizer, denoiserSizes.normalizerSize));
}

void PostProcessHandler::resize (uint32_t width, uint32_t height)
{
    beautyAccumBuffer.resize (width, height);
    albedoAccumBuffer.resize (width, height);
    normalAccumBuffer.resize (width, height);

    linearBeautyBuffer.resize (width * height);
    linearAlbedoBuffer.resize (width * height);
    linearNormalBuffer.resize (width * height);
    linearFlowBuffer.resize (width * height);
    linearDenoisedBeautyBuffer.resize (width * height);

    optixu::DenoiserSizes denoiserSizes;
    uint32_t numTasks;
    denoiser.prepare (
        width, height, tileWidth, tileHeight,
        &denoiserSizes, &numTasks);

    denoiserStateBuffer.resize (denoiserSizes.stateSize, 1);
    denoiserScratchBuffer.resize (std::max (denoiserSizes.scratchSize, denoiserSizes.scratchSizeForComputeNormalizer), 1);

    denoisingTasks.resize (numTasks);
    denoiser.getTasks (denoisingTasks.data());

    denoiser.setupState (ctx->cuStr, denoiserStateBuffer, denoiserScratchBuffer);
}

void PostProcessHandler::finalize()
{
    LOG (DBUG) << _FN_;

    linearDenoisedBeautyBuffer.finalize();
    linearFlowBuffer.finalize();
    linearNormalBuffer.finalize();
    linearAlbedoBuffer.finalize();
    linearBeautyBuffer.finalize();

    normalAccumBuffer.finalize();
    albedoAccumBuffer.finalize();
    beautyAccumBuffer.finalize();

    CUDADRV_CHECK (cuMemFree (hdrNormalizer));

    CUDADRV_CHECK (cuModuleUnload (moduleCopyBuffers));
    denoiserScratchBuffer.finalize();
    denoiserStateBuffer.finalize();
    denoiser.destroy();
}

void PostProcessHandler::tonemap()
{
    Eigen::Vector2i renderTarget = ctx->renderSize;
    uint32_t width = renderTarget.x();
    uint32_t height = renderTarget.y();

    cudau::dim3 dimPostProcessBuffers = kernelVisualizeToOutputBuffer.calcGridDim (width, height);
    kernelVisualizeToOutputBuffer (ctx->cuStr, dimPostProcessBuffers, linearDenoisedBeautyBuffer.getDevicePointer(), make_int2 (width, height));
}

void PostProcessHandler::denoise (bool newSequence)
{
    const Eigen::Vector2i& renderSize = ctx->renderSize;

    kernelCopyToLinearBuffers.launchWithThreadDim (
        ctx->cuStr, cudau::dim3 (renderSize.x(), renderSize.y()),
        beautyAccumBuffer.getSurfaceObject (0),
        albedoAccumBuffer.getSurfaceObject (0),
        normalAccumBuffer.getSurfaceObject (0),
        linearBeautyBuffer,
        linearAlbedoBuffer,
        linearNormalBuffer,
        uint2 (renderSize.x(), renderSize.y()));

    optixu::DenoiserInputBuffers inputBuffers = {};
    inputBuffers.noisyBeauty = linearBeautyBuffer;
    if (useAlbedo)
        inputBuffers.albedo = linearAlbedoBuffer;
    if (useNormal)
        inputBuffers.normal = linearNormalBuffer;
    inputBuffers.beautyFormat = OPTIX_PIXEL_FORMAT_FLOAT4;
    inputBuffers.albedoFormat = OPTIX_PIXEL_FORMAT_FLOAT4;
    inputBuffers.normalFormat = OPTIX_PIXEL_FORMAT_FLOAT4;

    denoiser.computeNormalizer (
        ctx->cuStr,
        linearBeautyBuffer, OPTIX_PIXEL_FORMAT_FLOAT4,
        denoiserScratchBuffer, hdrNormalizer);

    for (int i = 0; i < denoisingTasks.size(); ++i)
        denoiser.invoke (
            ctx->cuStr, denoisingTasks[i],
            inputBuffers, optixu::IsFirstFrame (newSequence),
            hdrNormalizer, 0.0f,
            linearDenoisedBeautyBuffer,
            nullptr, optixu::BufferView()); // no AOV outputs, no internal guide layer for the next frame
}

void PostProcessHandler::getRender (BufferToDisplay bufferTypeToDisplay)
{
    void* bufferToDisplay = nullptr;
    switch (bufferTypeToDisplay)
    {
        case BufferToDisplay::NoisyBeauty:
            bufferToDisplay = linearBeautyBuffer.getDevicePointer();
            break;
        case BufferToDisplay::Albedo:
            bufferToDisplay = linearAlbedoBuffer.getDevicePointer();
            break;
        case BufferToDisplay::Normal:
            bufferToDisplay = linearNormalBuffer.getDevicePointer();
            break;
        case BufferToDisplay::Flow:
            bufferToDisplay = linearFlowBuffer.getDevicePointer();
            break;
        case BufferToDisplay::DenoisedBeauty:
        {
            bufferToDisplay = linearDenoisedBeautyBuffer.getDevicePointer();

            float4* const renderedPixels = linearDenoisedBeautyBuffer.map();
            OIIO::ImageBuf& render = ctx->camera->getSensorPixels();

            std::memcpy (render.localpixels(), renderedPixels, render.spec().image_bytes());
            linearDenoisedBeautyBuffer.unmap();

            break;
        }

        default:
            // Assert_ShouldNotBeCalled();
            break;
    }
}

void PostProcessHandler::saveRender (uint32_t frameNumber)
{
    tonemap();
    getRender (BufferToDisplay::DenoisedBeauty);
    
    // FIXME

}

void PostProcessHandler::initializeScreenRelatedBuffers (uint32_t width, uint32_t height)
{
    beautyAccumBuffer.initialize2D (ctx->cuCtx, cudau::ArrayElementType::Float32, 4,
                                    cudau::ArraySurface::Enable, cudau::ArrayTextureGather::Disable,
                                    width, height, 1);
    albedoAccumBuffer.initialize2D (ctx->cuCtx, cudau::ArrayElementType::Float32, 4,
                                    cudau::ArraySurface::Enable, cudau::ArrayTextureGather::Disable,
                                    width, height, 1);
    normalAccumBuffer.initialize2D (ctx->cuCtx, cudau::ArrayElementType::Float32, 4,
                                    cudau::ArraySurface::Enable, cudau::ArrayTextureGather::Disable,
                                    width, height, 1);

    linearBeautyBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device,
                                   width * height);
    linearAlbedoBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device,
                                   width * height);
    linearNormalBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device,
                                   width * height);
    linearFlowBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device,
                                 width * height);
    linearDenoisedBeautyBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device,
                                           width * height);
}
