#pragma once

#include "MaterialHandler.h"
#include "GeometryHandler.h"
#include "SceneHandler.h"
#include "PipelineHandler.h"
#include "PostProcessHandler.h"
#include "TextureHandler.h"
#include "SkyDomeHandler.h"

struct Handlers
{
    Handlers (RenderContextPtr ctx)
    {
        scene = SceneHandler::create (ctx);
        mat = MaterialHandler::create (ctx);
        geo = GeometryHandler::create (ctx);
        pl = PipelineHandler::create (ctx);
        post = PostProcessHandler::create (ctx);
        texture = TextureHandler::create (ctx);
        skydome = SkyDomeHandler::create (ctx);
    }

    ~Handlers()
    {
        // FIXME is order important???
        scene.reset();
        mat.reset();
        geo.reset();
        pl.reset();
        texture.reset();
        skydome.reset();
    }

    SceneHandlerRef scene = nullptr;
    MaterialHandlerRef mat = nullptr;
    GeometryHandlerRef geo = nullptr;
    PipelineHandlerRef pl = nullptr;
    PostProcessHandlerRef post = nullptr;
    TextureHandlerRef texture = nullptr;
    SkyDomeHandlerRef skydome = nullptr;
};