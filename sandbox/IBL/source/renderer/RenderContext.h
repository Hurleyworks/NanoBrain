
#pragma once

#include "sabi_core/sabi_core.h"
#include "RenderUtilities.h"

// Forward declaration and type alias for a shared_ptr to RenderContext
using RenderContextPtr = std::shared_ptr<class RenderContext>;
using sabi::CameraHandle;

// Forward declaration for Handlers struct
struct Handlers;

class RenderContext : public std::enable_shared_from_this<RenderContext>
{
 public:
    // Returns shared_ptr to this object
    RenderContextPtr getPtr() { return shared_from_this(); }

    RenderContext() = default;
    ~RenderContext() = default;

    void init();
    void cleanup();

    CUcontext cuCtx;
    CUstream cuStr;
    optixu::Context optCtx;

    optixu::Scene scene;
    Eigen::Vector2i renderSize = DEFAULT_DESKTOP_WINDOW_SIZE;

    // Buffer for acceleration structure building
    cudau::Buffer asBuildScratchMem;

    // Pointer to Handlers object for handling different tasks
    std::unique_ptr<Handlers> handlers = nullptr;

    CameraHandle camera = nullptr;
    std::filesystem::path resourceFolder;
};
