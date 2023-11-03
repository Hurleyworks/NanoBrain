
#pragma once

#include "../scene/RenderableNode.h"
#include "PhysicsUtilities.h"
#include "PhysicsContext.h"

class NewtonEngine
{
 public:
    NewtonEngine();
    ~NewtonEngine();

    // Updates physics simulation based on the engine state
    bool update (PhysicsEngineState state);
    void addBody (OptiXWeakNode weakNode, PhysicsEngineState engineState);
    void addGeometryInstances (OptiXWeakNode instancedFrom, GeometryInstances& instances, PhysicsEngineState engineState);

 private:
    PhysicsContextPtr ctx = nullptr;

    ndFloat32 dGetElapsedSeconds();
    void dResetTimer();

    void resetEngine();

}; // end class NewtonEngine
