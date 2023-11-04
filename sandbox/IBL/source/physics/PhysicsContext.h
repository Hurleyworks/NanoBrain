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
