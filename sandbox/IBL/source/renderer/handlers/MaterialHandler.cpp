#include "MaterialHandler.h"
#include "../Shared.h"
#include "Handlers.h"

MaterialHandler::MaterialHandler (RenderContextPtr ctx) :
    ctx (ctx)
{
    LOG (DBUG) << _FN_;
}

MaterialHandler::~MaterialHandler()
{
    LOG (DBUG) << _FN_;
}

template <typename MaterialData>
optixu::Material MaterialHandler::createMaterial (const MaterialInfo& info, rapidobj::Material& material, const std::filesystem::path& materialFolder)
{
    // Retrieve pipeline using entry point type from the MaterialInfo struct.
    auto pl = ctx->handlers->pl->getPipeline (info.entryPoint);

    // Create a new Optix material.
    optixu::Material mat = ctx->optCtx.createMaterial();

    // Maybe set hit group for shading
    if (info.shadingProg != ProgramType::Invalid && info.rayTypeSearch != INVALID_RAY_TYPE)
        mat.setHitGroup (info.rayTypeSearch, pl->hitPrograms[info.shadingProg]);

    // Maybe set hit group for visibility
    if (info.visibilityProg != ProgramType::Invalid && info.rayTypeVisibility != INVALID_RAY_TYPE)
        mat.setHitGroup (info.rayTypeVisibility, pl->hitPrograms[info.visibilityProg]);

    MaterialData data = {};
    if (material.diffuse_texname != "")
    {
        std::filesystem::path imagePath (material.diffuse_texname);
        if (imagePath.is_absolute())
        {
            data.texture = ctx->handlers->texture->createCudaTextureFromImage (imagePath);
        }
        else
        {
            auto fullPath = FileServices::findFileInFolder (materialFolder, material.diffuse_texname);
            if (fullPath)
                data.texture = ctx->handlers->texture->createCudaTextureFromImage (fullPath.value());
        }
    }

    data.albedo = RGB (sRGB_degamma_s (material.diffuse[0]), sRGB_degamma_s (material.diffuse[1]), sRGB_degamma_s (material.diffuse[2]));

    // Set user data on the Optix material.
    mat.setUserData (data);

    return mat;
}

template <typename MaterialData>
optixu::Material MaterialHandler::createDefaultMaterial (const MaterialInfo& info)
{
    // Retrieve pipeline using entry point type from the MaterialInfo struct.
    auto pl = ctx->handlers->pl->getPipeline (info.entryPoint);

    // Create a new Optix material.
    optixu::Material mat = ctx->optCtx.createMaterial();

    // Maybe set hit group for shading
    if (info.shadingProg != ProgramType::Invalid && info.rayTypeSearch != INVALID_RAY_TYPE)
        mat.setHitGroup (info.rayTypeSearch, pl->hitPrograms[info.shadingProg]);

    // Maybe set hit group for visibility
    if (info.visibilityProg != ProgramType::Invalid && info.rayTypeVisibility != INVALID_RAY_TYPE)
        mat.setHitGroup (info.rayTypeVisibility, pl->hitPrograms[info.visibilityProg]);

    MaterialData data = {};
    data.albedo = RGB (sRGB_degamma_s (0.25f), sRGB_degamma_s (0.5f), sRGB_degamma_s (1.0f));

    // Set user data on the Optix material.
    mat.setUserData (data);

    return mat;
}

template optixu::Material MaterialHandler::createMaterial<Shared::MaterialData> (const MaterialInfo&, rapidobj::Material&, const std::filesystem::path&);
template optixu::Material MaterialHandler::createDefaultMaterial<Shared::MaterialData> (const MaterialInfo&);