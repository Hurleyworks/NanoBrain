
#include "NewtonWorld.h"
#include "NewtonCallbacks.h"
#include "handlers/NewtonHandlers.h"

#define MAX_PHYSICS_STEPS 1
#define MAX_PHYSICS_FPS 60.0f

// ctor
NewtonWorld::NewtonWorld (PhysicsContextPtr ctx) :
    ndWorld(),
    ctx (ctx)
{
    ClearCache();
}

// dtor
NewtonWorld::~NewtonWorld()
{
}

double NewtonWorld::advanceTime (ndFloat32 timestep)
{
    const ndFloat32 descreteStep = (1.0f / MAX_PHYSICS_FPS);

    if (acceleratedUpdate)
    {
        Update (descreteStep);
    }
    else
    {
        ndInt32 maxSteps = MAX_PHYSICS_STEPS;
        timeAccumulator += timestep;

        // if the time step is more than max timestep par frame, throw away the extra steps.
        if (timeAccumulator > descreteStep * (ndFloat32)maxSteps)
        {
            ndFloat32 steps = ndFloor (timeAccumulator / descreteStep) - (ndFloat32)maxSteps;
            ndAssert (steps >= 0.0f);
            timeAccumulator -= descreteStep * steps;
        }

        while (timeAccumulator > descreteStep)
        {
            Update (descreteStep);
            timeAccumulator -= descreteStep;
        }
    }
    if (ctx->synchronousPhysicsUpdate)
    {
        Sync();
    }
    return 0.0;
}

void NewtonWorld::NormalUpdates()
{
    acceleratedUpdate = false;
}

void NewtonWorld::AccelerateUpdates()
{
    acceleratedUpdate = true;
}

void NewtonWorld::PreUpdate (ndFloat32 timestep)
{
    // LOG (DBUG) << _FN_;
}

void NewtonWorld::PostUpdate (ndFloat32 timestep)
{
    ctx->handlers->body->onPostUpdate (timestep);
}
