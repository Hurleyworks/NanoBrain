#include "PipelineHandler.h"

// Constructor: Initializes the rendering context and logs the function name
PipelineHandler::PipelineHandler (RenderContextPtr ctx) :
    ctx (ctx)
{
    LOG (DBUG) << _FN_;
}

// Destructor: Cleans up resources tied to the pipeline and logs the function name
PipelineHandler::~PipelineHandler()
{
    LOG (DBUG) << _FN_;

    // Iterate through all pipelines and finalize and destroy resources
    for (auto& it : pipelines)
    {
        auto& pl = it.second;
        pl->sceneDependentSBT.finalize();
        pl->shaderBindingTable.finalize();
        for (int i = 0; i < pl->callablePrograms.size(); ++i)
            pl->callablePrograms[i].destroy();
        for (auto& pair : pl->programs)
            pair.second.destroy();
        for (auto& pair : pl->hitPrograms)
            pair.second.destroy();
        for (auto& pair : pl->entryPoints)
            pair.second.destroy();
        pl->optixModule.destroy();
        pl->optixPipeline.destroy();
    }
}

// Configures the pipeline using the data provided in the PipelineData object
void PipelineHandler::setupPipeline (const PipelineData& data)
{
    std::filesystem::path& resourcePath = ctx->resourceFolder;
    std::filesystem::path ptxFile = resourcePath / "ptx" / "optix_kernels.optixir";

    // Read the binary PTX file
    const std::vector<char> optixIr = readBinaryFile (ptxFile);
    if (optixIr.size() == 0)
        throw std::runtime_error ("ptxFile failed to load");

    auto pl = getPipeline (data.entryPoint);
    optixu::Pipeline& opl = pl->optixPipeline;
    optixu::Module& mod = pl->optixModule;
    opl = ctx->optCtx.createPipeline();

    // Set pipeline options
    opl.setPipelineOptions (
        data.numPayloadValuesInDwords,
        data.numAttributeValuesInDwords,
        data.plpName.c_str(), data.sizeOfLaunchParams,
        OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_LEVEL_INSTANCING,
        OPTIX_EXCEPTION_FLAG_STACK_OVERFLOW | OPTIX_EXCEPTION_FLAG_TRACE_DEPTH,
        OPTIX_PRIMITIVE_TYPE_FLAGS_TRIANGLE);

    // Create Optix module from IR
    mod = opl.createModuleFromOptixIR (
        optixIr, OPTIX_COMPILE_DEFAULT_MAX_REGISTER_COUNT,
        DEBUG_SELECT (OPTIX_COMPILE_OPTIMIZATION_LEVEL_0, OPTIX_COMPILE_OPTIMIZATION_DEFAULT),
        DEBUG_SELECT (OPTIX_COMPILE_DEBUG_LEVEL_FULL, OPTIX_COMPILE_DEBUG_LEVEL_NONE));

    // Create miss, ray gen, and hit programs
    pl->emptyMissProgram = opl.createMissProgram (pl->emptyModule, nullptr);
    pl->entryPoints[data.entryPoint] = opl.createRayGenProgram (mod, data.rayGenName.c_str());
    pl->emptyMissProgram = opl.createMissProgram (pl->emptyModule, nullptr);

    pl->entryPoints[data.entryPoint] = opl.createRayGenProgram (
        mod, data.rayGenName.c_str());

    pl->hitPrograms[ProgramType::shading] = opl.createHitProgramGroupForTriangleIS (
        mod, data.closestHitName.c_str(),
        pl->emptyModule, nullptr);

    pl->hitPrograms[ProgramType::visibility] = opl.createHitProgramGroupForTriangleIS (
        pl->emptyModule, nullptr,
        mod, data.anyHitName.c_str());

    pl->programs[ProgramType::miss] = opl.createMissProgram (mod, data.missName.c_str());

    // Link and set the pipeline
    // Trace depth is 2 because this sample trace rays from the ray
    // generation program. and from the closest hit program
    uint32_t traceDepth = 2;
    opl.link (traceDepth);
    opl.setNumMissRayTypes (data.numOfRayTypes);
    opl.setMissProgram (data.searchRay, pl->programs[ProgramType::miss]);
    opl.setMissProgram (data.visibilityRay, pl->emptyMissProgram);
    pl->setEntryPoint (data.entryPoint);

    opl.setScene (ctx->scene);
}

// Sets up the Shader Binding Table (SBT) using the EntryPointType
void PipelineHandler::setupSBT (EntryPointType type)
{
    auto pl = getPipeline (type);
    optixu::Pipeline& opl = pl->optixPipeline;

    size_t sbtSize;
    opl.generateShaderBindingTableLayout (&sbtSize);
    pl->shaderBindingTable.initialize (ctx->cuCtx, cudau::BufferType::Device, sbtSize, 1);
    pl->shaderBindingTable.setMappedMemoryPersistent (true);
    opl.setShaderBindingTable (pl->shaderBindingTable, pl->shaderBindingTable.getMappedPointer());
}

// Sets up a scene-dependent Shader Binding Table (SBT)
void PipelineHandler::setSceneDependentSBT (EntryPointType type)
{
    auto pl = getPipeline (EntryPointType::pathtrace);
    optixu::Pipeline& opl = pl->optixPipeline;

    opl.setHitGroupShaderBindingTable (pl->sceneDependentSBT, pl->sceneDependentSBT.getMappedPointer());
}

