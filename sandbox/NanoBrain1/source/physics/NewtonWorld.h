
#pragma once

#include "PhysicsUtilities.h"
#include "PhysicsContext.h"

class NewtonWorld : public ndWorld
{
 public:
    NewtonWorld (PhysicsContextPtr ctx);
    ~NewtonWorld();

    double advanceTime (ndFloat32 timestep);
    void NormalUpdates();
    void AccelerateUpdates();

 private:
    PhysicsContextPtr ctx = nullptr;
    ndFloat32 timeAccumulator = 0.0f;
    bool acceleratedUpdate = false;

    void PreUpdate (ndFloat32 timestep) override;
    void PostUpdate (ndFloat32 timestep) override;
};
