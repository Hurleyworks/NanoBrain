

#include "NewtonContactHandler.h"
#include "../NewtonWorld.h"

NewtonContactHandler::NewtonContactHandler (PhysicsContextPtr ctx) :
    ndContactNotify (ctx->newtonWorld->GetScene()),
    ctx (ctx)
{
}

NewtonContactHandler::~NewtonContactHandler()
{
}

void NewtonContactHandler::OnBodyAdded (ndBodyKinematic* const) const
{
    //LOG (DBUG) << _FN_;
}

void NewtonContactHandler::OnBodyRemoved (ndBodyKinematic* const) const
{
}

void NewtonContactHandler::OnContactCallback (const ndContact* const, ndFloat32) const
{
    //LOG (DBUG) << _FN_;
}

bool NewtonContactHandler::OnAabbOverlap (const ndContact* const contact, ndFloat32 timestep) const
{
   // LOG (DBUG) << _FN_;
    return true;
}

bool NewtonContactHandler::OnCompoundSubShapeOverlap (const ndContact* const contact, ndFloat32 timestep, const ndShapeInstance* const subShapeA, const ndShapeInstance* const subShapeB) const
{
    //LOG (DBUG) << _FN_;
    return true;
}
