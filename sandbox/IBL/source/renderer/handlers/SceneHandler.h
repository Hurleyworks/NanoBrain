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
