#pragma once

#include "../RenderContext.h"

using MaterialHandlerRef = std::shared_ptr<class MaterialHandler>;

class MaterialHandler
{
 public:
    // Factory function to create and return a shared pointer to a MaterialHandler.
    static MaterialHandlerRef create (RenderContextPtr ctx) { return std::make_shared<MaterialHandler> (ctx); }

 public:
    MaterialHandler (RenderContextPtr ctx);
    ~MaterialHandler();

    template <typename MaterialData>
    optixu::Material createMaterial (const MaterialInfo& info, rapidobj::Material& material, const std::filesystem::path& materialFolder);

    template <typename MaterialData>
    optixu::Material createMaterial (const MaterialInfo& info, const cgltf_material& material, const std::filesystem::path& materialFolder);

    template <typename MaterialData>
    optixu::Material createDefaultMaterial (const MaterialInfo& info);

 private:
    RenderContextPtr ctx = nullptr;
};
