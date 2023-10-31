#include "SceneHandler.h"
#include "Handlers.h"

SceneHandler::SceneHandler (RenderContextPtr ctx) :
    ctx (ctx)
{
    LOG (DBUG) << _FN_;
}

SceneHandler::~SceneHandler()
{
    LOG (DBUG) << _FN_;

    // clear the nodes map which will destroy all optixu::Instances
    nodes.clear();

    if (travHandle != 0)
    {
        // Finalize the instance buffer and IAS memory
        instanceBuffer.finalize();
        iasMem.finalize();

        // Destroy the IAS
        ias.destroy();
    }
}

// Initialize the IAS with preferred settings
void SceneHandler::init()
{
    // Create Instance Acceleration Structure (IAS)
    ias = ctx->scene.createInstanceAccelerationStructure();

    // Set the trade-off for the IAS to prefer fast trace
    ias.setConfiguration (optixu::ASTradeoff::PreferFastTrace);
}

// Create and add an instance to the scene
void SceneHandler::createInstance (OptiXNode node, EntryPointType type)
{
    if (travHandle == 0)
        init();

    // Create a new OptiX instance
    optixu::Instance instance = ctx->scene.createInstance();

    // Set the Geometry Acceleration Structure (GAS)
    instance.setChild (node->g->getGAS().gas);

    // Set the instance transform using the given pose
    const Eigen::Matrix4f& m = node->st.worldTransform.matrix();
    MatrixRowMajor34f t = m.block<3, 4> (0, 0);
    instance.setTransform (t.data());

    // Add the instance to the IAS
    ias.addChild (instance);

    node->instance = instance;
    node->iasIndex = ias.findChildIndex (instance);

    nodes[node->name] = node;

    GAS& gasData = node->g->getGAS();
    gasData.gas.rebuild (ctx->cuStr, gasData.gasMem, ctx->asBuildScratchMem);

    prepareForBuild();

    rebuild (type);
}

void SceneHandler::createGeometryInstances (GeometryInstances& instances, EntryPointType type)
{
    for (auto& node : instances)
    {
        if (node->instancedFrom.expired()) continue;

        OptiXNode instancedFrom = node->instancedFrom.lock();

        // Create a new OptiX instance
        optixu::Instance instance = ctx->scene.createInstance();

        // Set the Geometry Acceleration Structure (GAS)
        // using  the instancedFrom node
        instance.setChild (instancedFrom->g->getGAS().gas);

        // Set the instance transform using the given pose
        const Eigen::Matrix4f& m = node->st.worldTransform.matrix();
        MatrixRowMajor34f t = m.block<3, 4> (0, 0);
        instance.setTransform (t.data());

        // Add the instance to the IAS
        ias.addChild (instance);

        node->instance = instance;
        node->iasIndex = ias.findChildIndex (instance);

        nodes[node->name] = node;
    }

    prepareForBuild();

    rebuild (type);
}

void SceneHandler::removeNode (OptiXNode node)
{
    auto it = nodes.find (node->name);
    if (it == nodes.end()) return;

    optixu::Instance instance = ias.getChild (node->iasIndex);

    // this might change the iasIndex
    // of other nodes
    ias.removeChildAt (node->iasIndex);

    // remove this node from the nodes map and the
    // reference counted node will self destruct, cleaning
    // up it's geometry and destroying it's instance
    nodes.erase (it);

    // update the iasIndex of remaining nodes because
    // the index might have changed
    for (auto it : nodes)
    {
        auto node = it.second;
        if (!node) continue;

        node->iasIndex = ias.findChildIndex (node->instance);
    }
}

void SceneHandler::updateMotion()
{
    if (!nodes.size()) return;

    for (auto& it : nodes)
    {
        OptiXNode node = it.second;

        // Set the instance transform using the given pose
        const Eigen::Matrix4f& m = node->st.worldTransform.matrix();
        MatrixRowMajor34f t = m.block<3, 4> (0, 0);
        node->instance.setTransform (t.data());
    }

    rebuildIAS();
}

// Prepare for building the IAS
void SceneHandler::prepareForBuild()
{
    // Prepare the IAS for build and get memory requirements
    OptixAccelBufferSizes bufferSizes;
    ias.prepareForBuild (&bufferSizes);

    if (bufferSizes.tempSizeInBytes > ctx->asBuildScratchMem.sizeInBytes())
        ctx->asBuildScratchMem.resize (bufferSizes.tempSizeInBytes, 1, ctx->cuStr);

    if (iasMem.isInitialized())
    {
        CUDADRV_CHECK (cuStreamSynchronize (ctx->cuStr));
        iasMem.resize (bufferSizes.outputSizeInBytes, 1, ctx->cuStr);

        if (ias.getNumChildren())
            instanceBuffer.resize (ias.getNumChildren());
    }
    else
    {
        // Initialize memory buffers based on requirements
        CUDADRV_CHECK (cuStreamSynchronize (ctx->cuStr));
        iasMem.initialize (ctx->cuCtx, cudau::BufferType::Device, bufferSizes.outputSizeInBytes, 1);
        instanceBuffer.initialize (ctx->cuCtx, cudau::BufferType::Device, ias.getNumChildren());
    }
}

// Initialize Scene Dependent Shader Binding Table (SBT)
void SceneHandler::initializeSceneDependentSBT (EntryPointType type)
{
    // Get the pipeline for  entry point
    auto pipeline = ctx->handlers->pl->getPipeline (type);

    // Get the size of the Hit Group SBT
    size_t hitGroupSbtSize;
    ctx->scene.generateShaderBindingTableLayout (&hitGroupSbtSize);

    // Initialize the scene dependent SBT
    pipeline->sceneDependentSBT.initialize (ctx->cuCtx, cudau::BufferType::Device, hitGroupSbtSize, 1);

    // Keep the mapped memory persistent
    pipeline->sceneDependentSBT.setMappedMemoryPersistent (true);
}

// Resize scene dependent Shader Binding Table (SBT)
void SceneHandler::resizeSceneDependentSBT (EntryPointType type)
{
    // Get the pipeline for entry point
    auto pipeline = ctx->handlers->pl->getPipeline (type);

    // Get the size of the Hit Group SBT
    size_t hitGroupSbtSize;
    ctx->scene.generateShaderBindingTableLayout (&hitGroupSbtSize);

    // resizew the Hit Group SBT
    pipeline->sceneDependentSBT.resize (hitGroupSbtSize, 1, ctx->cuStr);

    // set the scene dependent SBT
    ctx->handlers->pl->setSceneDependentSBT (type);
}

// Rebuild the IAS after any updates to the instances
void SceneHandler::rebuildIAS()
{
    // Perform the IAS rebuild
    travHandle = ias.rebuild (ctx->cuStr, instanceBuffer, iasMem, ctx->asBuildScratchMem);

    // Synchronize the CUDA stream to ensure completion
    CUDADRV_CHECK (cuStreamSynchronize (ctx->cuStr));
}

void SceneHandler::rebuild (EntryPointType type)
{
    resizeSceneDependentSBT (type);
    rebuildIAS();
}
