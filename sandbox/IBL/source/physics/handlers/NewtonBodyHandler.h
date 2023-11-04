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
