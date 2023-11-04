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