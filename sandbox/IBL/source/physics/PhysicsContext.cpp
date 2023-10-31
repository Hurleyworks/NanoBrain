#include "PhysicsContext.h"
#include "handlers/NewtonHandlers.h"
#include "NewtonWorld.h"

void PhysicsContext::init()
{
    // must precede handlers creation!
    newtonWorld = std::make_unique<NewtonWorld> (getPtr());

    handlers = std::make_unique<NewtonHandlers> (getPtr());

    newtonWorld->SetSubSteps (solverSubSteps);
    newtonWorld->SetSolverIterations (solverPasses);
    newtonWorld->SetThreadCount (workerThreads);
    newtonWorld->SelectSolver (solverMode);

    newtonWorld->SetContactNotify (handlers->contact);
}