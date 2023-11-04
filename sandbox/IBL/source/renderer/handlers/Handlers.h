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