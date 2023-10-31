#pragma once

#include "../RenderContext.h"
#include "../../scene/RenderableNode.h"

// Forward declaration
using SceneHandlerRef = std::shared_ptr<class SceneHandler>;

class SceneHandler
{
 public:
    // Factory function for creating SceneHandler objects
    static SceneHandlerRef create (RenderContextPtr ctx) { return std::make_shared<SceneHandler> (ctx); }

    using NodeMap = std::unordered_map<std::string, OptiXNode>;

 public:
    SceneHandler (RenderContextPtr ctx);
    ~SceneHandler();

    void createInstance (OptiXNode node, EntryPointType type);
    void createGeometryInstances (GeometryInstances& instances, EntryPointType type);

    void removeNode (const std::string& nodeName, EntryPointType type)
    {
        auto it = nodes.find (nodeName);
        if (it != nodes.end())
            removeNode (it->second);
        else
            return;

        resizeSceneDependentSBT (type);
        prepareForBuild();
        rebuildIAS();
    }

    void updateMotion();

    // Prepare Instance Acceleration Structure (IAS) for build
    void prepareForBuild();

    // Initialize Scene Dependent Shader Binding Table (SBT)
    void initializeSceneDependentSBT (EntryPointType type);

    // Resize Scene Dependent Shader Binding Table (SBT)
    void resizeSceneDependentSBT (EntryPointType type);

    // Rebuild the IAS after updates
    void rebuildIAS();

    void rebuild (EntryPointType type);

    // Get traversable handle for the scene
    OptixTraversableHandle getHandle() { return travHandle; }

 private:
    // Reference to the render ctx
    RenderContextPtr ctx = nullptr;

    void removeNode (OptiXNode node);

    NodeMap nodes;

    // Instance Acceleration Structure (IAS)
    optixu::InstanceAccelerationStructure ias;

    // Buffer for IAS memory
    cudau::Buffer iasMem;

    // Typed buffer for OptixInstance
    cudau::TypedBuffer<OptixInstance> instanceBuffer;

    // Traversable handle for the scene
    OptixTraversableHandle travHandle = 0;

    // Initialize the scene
    void init();
};
