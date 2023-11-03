#pragma once

#include "../renderer/geometry/OptiXGeometry.h"
#include "../physics/PhysicsUtilities.h"

#include "SpaceTime.h"

// Alias for a shared pointer to the OptiXRenderable class
using OptiXNode = std::shared_ptr<class OptiXRenderable>;
using OptiXWeakNode = std::weak_ptr<class OptiXRenderable>;

class OptiXRenderable
{
 public:
    // Factory function for creating OptiXRenderable objects
    static OptiXNode create() { return std::make_shared<OptiXRenderable>(); }

 public:
    OptiXRenderable();
    ~OptiXRenderable();

    void* getUserdata() { return userdata; }
    void setUserData (void* const data) { userdata = data; }

    // geometry instances have no geometry
    bool isInstance() { return g == nullptr; }

    // static bodies have no mass
    bool isStaticBody() { return desc.mass == 0.0f; }

    SpaceTime st;
    OptiXGeometryRef g = nullptr;
    PhysicsDesc desc;

    optixu::Instance instance;
    uint32_t iasIndex = 0; // index into IAS children
    std::string name = "unnamed_node";

    OptiXWeakNode instancedFrom;

    void* userdata = nullptr;
};

using RenderableStack = moodycamel::ConcurrentQueue<OptiXNode>; // threadsafe queue
using GeometryInstances = std::vector<OptiXNode>;