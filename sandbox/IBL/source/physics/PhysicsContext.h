#pragma once

#include "sabi_core/sabi_core.h"
#include <ndNewton.h>
#include "../scene/RenderableNode.h"

// Forward declaration and type alias for a shared_ptr to PhysicsContext
using PhysicsContextPtr = std::shared_ptr<class PhysicsContext>;

// Forward declaration for Handlers struct
struct NewtonHandlers;
class NewtonWorld;

class PhysicsContext : public std::enable_shared_from_this<PhysicsContext>
{
 public:
    using WeakNodes = std::vector<OptiXWeakNode>;

 public:
    // Returns shared_ptr to this object
    PhysicsContextPtr getPtr() { return shared_from_this(); }

    PhysicsContext() = default;
    ~PhysicsContext() = default;

    void init();

    // Pointer to Handlers object for handling different tasks
    std::unique_ptr<NewtonHandlers> handlers = nullptr;

    std::unique_ptr<NewtonWorld> newtonWorld = nullptr;
    bool synchronousPhysicsUpdate = false;

    int solverPasses = 4;
    int solverSubSteps = 2;
    int workerThreads = 10;
    ndWorld::ndSolverModes solverMode = ndWorld::ndSimdAvx2Solver;

    WeakNodes weakNodes;
};
