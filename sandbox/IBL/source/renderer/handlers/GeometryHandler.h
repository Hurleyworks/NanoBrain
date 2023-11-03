#pragma once

#include "../geometry/OptiXGeometry.h"

// Alias for shared_ptr to GeometryHandler class
using GeometryHandlerRef = std::shared_ptr<class GeometryHandler>;

class GeometryHandler
{
 public:
    // Factory function for creating a GeometryHandler
    static GeometryHandlerRef create (RenderContextPtr ctx) { return std::make_shared<GeometryHandler> (ctx); }

 public:
    // Constructors and Destructor
    GeometryHandler (RenderContextPtr ctx);
    ~GeometryHandler();

 private:
    // Reference to the RenderContext
    RenderContextPtr ctx = nullptr;
};
