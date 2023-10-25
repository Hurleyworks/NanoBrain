
#pragma once

#include "../PhysicsContext.h"
#include "../PhysicsUtilities.h"
#include "../../scene/RenderableNode.h"

// Forward declaration
using NewtonBodyHandlerRef = std::shared_ptr<class NewtonBodyHandler>;

class NewtonBodyHandler
{
 public:
    static NewtonBodyHandlerRef create (PhysicsContextPtr ctx) { return std::make_shared<NewtonBodyHandler> (ctx); }

 public:
    NewtonBodyHandler (PhysicsContextPtr ctx);
    ~NewtonBodyHandler();

    void addBody (OptiXWeakNode weakNode, PhysicsEngineState engineState);
    void addGeometryInstances (OptiXWeakNode instancedFrom, GeometryInstances& instances, PhysicsEngineState engineState);

    void onPostUpdate (ndFloat32 timestep);

 private:
    PhysicsContextPtr ctx = nullptr;

    // must use thread safe containers when engine is running because Newton's
    // thread may be popping while the addNewBody thread is pushing
    RenderableStack pendingAdds;
    RenderableStack pendingGeometryInstances;
    RenderableStack pendingPoses;
    RenderableStack pendingRemoves;
    RenderableStack pendingPropertyUpdates;

    void addBodyToEngine (OptiXWeakNode weakNode);
    void addGeometryInstanceToEngine (OptiXWeakNode instanceFrom, OptiXWeakNode weakNode);

}; // end class NewtonBodyHandler
