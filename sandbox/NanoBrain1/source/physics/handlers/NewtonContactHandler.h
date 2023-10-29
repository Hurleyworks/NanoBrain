
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
