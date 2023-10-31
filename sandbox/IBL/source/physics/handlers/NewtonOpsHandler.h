
#pragma once

#include "../PhysicsContext.h"
#include "../../scene/RenderableNode.h"
#include <ndNewton.h>

// Forward declaration
using NewtonOpsHandlerRef = std::shared_ptr<class NewtonOpsHandler>;

class NewtonOpsHandler
{
 public:
    static NewtonOpsHandlerRef create (PhysicsContextPtr ctx) { return std::make_shared<NewtonOpsHandler> (ctx); }

 public:
    NewtonOpsHandler (PhysicsContextPtr ctx);
    ~NewtonOpsHandler();

    void createConvexHullMesh (OptiXWeakNode weakNode);
    ndShapeInstance createCollisionShape (OptiXWeakNode weakNode);

 private:
    PhysicsContextPtr ctx = nullptr;

}; // end class NewtonOpsHandler
