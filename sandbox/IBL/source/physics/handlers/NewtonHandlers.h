
#pragma once

#include "../../scene/RenderableNode.h"

#include "NewtonBodyHandler.h"
#include "NewtonContactHandler.h"
#include "NewtonOpsHandler.h"

struct NewtonHandlers
{
    NewtonHandlers (PhysicsContextPtr ctx)
    {
        body = NewtonBodyHandler::create (ctx);
        ops = NewtonOpsHandler::create (ctx);

        // owned by Newton don't delete
        contact = new NewtonContactHandler (ctx);
    }

    ~NewtonHandlers()
    {
    }

    NewtonBodyHandlerRef body = nullptr;
    NewtonOpsHandlerRef ops = nullptr;
    NewtonContactHandler* contact = nullptr; // owned by Newton don't delete
};
