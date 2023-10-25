#include "GeometryHandler.h"
#include "Handlers.h"

GeometryHandler::GeometryHandler (RenderContextPtr ctx) :
    ctx (ctx)
{
    LOG (DBUG) << _FN_;
}

GeometryHandler::~GeometryHandler()
{
    
}

