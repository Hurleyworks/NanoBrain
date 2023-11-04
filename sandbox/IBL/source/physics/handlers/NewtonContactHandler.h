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
#include <ndNewton.h>

// Forward declaration
using NewtonContactHandlerRef = std::shared_ptr<class NewtonContactHandler>;

class NewtonContactHandler : public ndContactNotify
{
 public:
    static NewtonContactHandlerRef create (PhysicsContextPtr ctx) { return std::make_shared<NewtonContactHandler> (ctx); }

 public:
    NewtonContactHandler (PhysicsContextPtr ctx);
    ~NewtonContactHandler();

    void OnBodyAdded (ndBodyKinematic* const) const override;
    void OnBodyRemoved (ndBodyKinematic* const) const override;
    void OnContactCallback (const ndContact* const, ndFloat32) const override;
    bool OnAabbOverlap (const ndContact* const contact, ndFloat32 timestep) const override;
    bool OnCompoundSubShapeOverlap (const ndContact* const contact, ndFloat32 timestep, const ndShapeInstance* const subShapeA, const ndShapeInstance* const subShapeB) const override;

 private:
    PhysicsContextPtr ctx = nullptr;
};
