#include "RenderContext.h"
#include "Shared.h"
#include "handlers/Handlers.h"

void RenderContext::init()
{
    CUDADRV_CHECK (cuInit (0));
    CUDADRV_CHECK (cuCtxCreate (&cuCtx, 0, 0));
    CUDADRV_CHECK (cuCtxSetCurrent (cuCtx));
    CUDADRV_CHECK (cuStreamCreate (&cuStr, 0));

    optCtx = optixu::Context::create (
        cuCtx, 4,
        optixu::EnableValidation::DEBUG_SELECT (Yes, No));

    handlers = std::make_unique<Handlers> (getPtr());

    scene = optCtx.createScene();

    asBuildScratchMem.initialize (cuCtx, cudau::BufferType::Device, 32 * 1024 * 1024, 1);
}

void RenderContext::cleanup()
{
    try
    {
        handlers.reset();

        scene.destroy();

        asBuildScratchMem.finalize();

        optCtx.destroy();

        CUDADRV_CHECK (cuStreamDestroy (cuStr));
        CUDADRV_CHECK (cuCtxDestroy (cuCtx));
    }
    catch (std::exception& e)
    {
        LOG (CRITICAL) << e.what();
    }
}

