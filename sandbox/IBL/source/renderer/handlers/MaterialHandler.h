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
