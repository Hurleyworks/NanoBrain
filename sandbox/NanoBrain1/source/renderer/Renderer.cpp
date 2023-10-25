#include "Renderer.h"

using Eigen::Vector3f;

Renderer::~Renderer()
{
    try
    {
        // Free CUDA memory for pipeline launch parameters
        CUDADRV_CHECK (cuMemFree (plpOnDevice));

        // Finalize random number generator
        rngBuffer.finalize();

        // Cleanup the render ctx
        ctx->cleanup();
    }
    catch (std::exception& e)
    {
        LOG (CRITICAL) << e.what();
    }
}

void Renderer::init (const std::filesystem::path& resourceFolder)
{
    try
    {
        // Initialize render ctx
        ctx = std::make_shared<RenderContext>();
        ctx->init();

        ctx->resourceFolder = resourceFolder;

        // Initialize the random number generator
        rngBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device, ctx->renderSize.x(), ctx->renderSize.y());
        {
            std::mt19937_64 rng (591842031321323413);

            rngBuffer.map();
            for (int y = 0; y < ctx->renderSize.y(); ++y)
                for (int x = 0; x < ctx->renderSize.x(); ++x)
                    rngBuffer (x, y).setState (rng());
            rngBuffer.unmap();
        };

        // Setup pipeline and Shader Binding Table
        PipelineData data;
        data.rayGenName = rt_raygen_name_str ("pathTracing");
        data.anyHitName = rt_ah_name_str ("visibility");
        data.closestHitName = rt_ch_name_str ("shading");
        data.missName = rt_miss_name_str ("miss");
        data.numOfRayTypes = Shared::RayType::NumRayTypes;
        data.searchRay = Shared::RayType::RayType_Search;
        data.visibilityRay = Shared::RayType::RayType_Visibility;
        data.entryPoint = EntryPointType::pathtrace;
        data.numPayloadValuesInDwords = std::max (Shared::SearchRayPayloadSignature::numDwords,
                                                  Shared::VisibilityRayPayloadSignature::numDwords);
        data.numAttributeValuesInDwords = optixu::calcSumDwords<float2>();
        data.plpName = "plp";
        data.sizeOfLaunchParams = sizeof (Shared::PipelineLaunchParameters);
        ctx->handlers->pl->setupPipeline (data);

        ctx->handlers->pl->setupSBT (EntryPointType::pathtrace);

        // init PostProcess handler
        ctx->handlers->post->initialize();

        ctx->handlers->scene->initializeSceneDependentSBT (EntryPointType::pathtrace);

        // Set the scene dependent SBT
        ctx->handlers->pl->setSceneDependentSBT (EntryPointType::pathtrace);

        // set plp parameters
        plp.travHandle = 0;
        plp.imageSize.x = ctx->renderSize.x();
        plp.imageSize.y = ctx->renderSize.y();
        plp.colorAccumBuffer = ctx->handlers->post->getBeautyBuffer();
        plp.albedoAccumBuffer = ctx->handlers->post->getAlbedoBuffer();
        plp.normalAccumBuffer = ctx->handlers->post->getNormalBuffer();
        plp.rngBuffer = rngBuffer.getBlockBuffer2D();
        plp.useCameraSpaceNormal = 1;

        // skydome
        plp.envLightRotation = envLightRotation;
        plp.envLightPowerCoeff = log10EnvLightPowerCoeff;
        plp.enableEnvLight = 0;
        plp.envLightTexture = 0;

        renderedPixels = std::make_unique<float4[]> (ctx->renderSize.x() * ctx->renderSize.y());

        // Allocate pipeline launch parameters to device
        CUDADRV_CHECK (cuMemAlloc (&plpOnDevice, sizeof (plp)));
    }
    catch (std::exception& e)
    {
        LOG (CRITICAL) << e.what();
    }
}
void Renderer::setCamera (CameraHandle camera)
{
    ctx->camera = camera;
}
void Renderer::render()
{
    try
    {
        // get OptixTraversableHandle
        plp.travHandle = ctx->handlers->scene->getHandle();
        plp.envLightPowerCoeff = std::pow (10.0f, log10EnvLightPowerCoeff);

        if (restartRender)
        {
            restartRender = false;
            numAccumFrames = 0;
        }

        // maybe update camera and reset numAccumFrames
        if (ctx->camera->isDirty())
        {
            updateCamera (ctx->camera);
            numAccumFrames = 0;
        }

        plp.numAccumFrames = numAccumFrames;

        // Copy pipeline launch parameters to device
        CUDADRV_CHECK (cuMemcpyHtoDAsync (plpOnDevice, &plp, sizeof (plp), ctx->cuStr));

        // Launch pipeline
        ctx->handlers->pl->getPipeline (EntryPointType::pathtrace)->optixPipeline.launch (ctx->cuStr, plpOnDevice, ctx->renderSize.x(), ctx->renderSize.y(), 1);

        // Synchronize CUDA stream
        CUDADRV_CHECK (cuStreamSynchronize (ctx->cuStr));

        // Denoise and get render
        ctx->handlers->post->denoise (numAccumFrames == 0);
        ctx->handlers->post->getRender (BufferToDisplay::DenoisedBeauty);

        ++numAccumFrames;
    }
    catch (std::exception& e)
    {
        LOG (CRITICAL) << e.what();
    }
}

void Renderer::addSkyDomeImage (const OIIO::ImageBuf&& image)
{
    // FIXME
    if (image.spec().nchannels != 3)
    {
        return;
    }

    ctx->handlers->skydome->addSkyDomeImage (std::move (image));
    plp.envLightTexture = ctx->handlers->skydome->getSkyDomeTexture();
    plp.enableEnvLight = 1;
}

void Renderer::addRenderableNode (OptiXNode node, const std::filesystem::path& path)
{
    MaterialInfo materialInfo;
    materialInfo.entryPoint = EntryPointType::pathtrace;
    materialInfo.shadingProg = ProgramType::shading;
    materialInfo.visibilityProg = ProgramType::visibility;
    materialInfo.rayTypeSearch = Shared::RayType_Search;
    materialInfo.rayTypeVisibility = Shared::RayType_Visibility;

    node->g->fromFile (path);
    node->g->createGeometry (ctx, node->st, materialInfo);
    node->g->createGAS (ctx, Shared::NumRayTypes);

    ctx->handlers->scene->createInstance (node, EntryPointType::pathtrace);

    restartRender = true;
}

void Renderer::addRenderableGeometryInstances (OptiXNode instancedFrom, GeometryInstances& instances)
{
    float stackOffset = 1.0f;

    // make a stack of geometry instances
    for (int i = 0; i < instances.size(); i++)
    {
        OptiXNode node = OptiXRenderable::create();
        node->instancedFrom = instancedFrom;
        node->name = instancedFrom->name + "_instance_" + std::to_string (i);

        node->st.worldTransform.setIdentity();
        node->st.worldTransform.translation().y() += stackOffset;
        node->st.makeCurrentPoseStartPose();
        node->desc = instancedFrom->desc;

        stackOffset += 1.0f;

        instances[i] = node;
    }
    ctx->handlers->scene->createGeometryInstances (instances, EntryPointType::pathtrace);

    restartRender = true;
}

void Renderer::removeRenderableNode (const std::string& name)
{
    ctx->handlers->scene->removeNode (name, EntryPointType::pathtrace);

    // Set the scene dependent SBT
    ctx->handlers->pl->setSceneDependentSBT (EntryPointType::pathtrace);

    restartRender = true;
}

void Renderer::updateMotion()
{
    ctx->handlers->scene->updateMotion();
    restartRender = true;
}
