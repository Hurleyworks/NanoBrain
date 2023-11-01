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

// much taken from Shocker GfxExp
// https://github.com/shocker-0x15/GfxExp

#include <common_host.h>
#include "../RenderContext.h"

using SkyDomeHandlerRef = std::shared_ptr<class SkyDomeHandler>;

class SkyDomeHandler
{
 public:
    static SkyDomeHandlerRef create (RenderContextPtr ctx) { return std::make_shared<SkyDomeHandler> (ctx); }
    using LoadedSkyDomes = std::unordered_map<std::string, OIIO::ImageBuf>;

 public:
    SkyDomeHandler (RenderContextPtr ctx);
    ~SkyDomeHandler();

    void addSkyDomeImage (const OIIO::ImageBuf&& image);
     
    CUtexObject getEviroTexture() { return envLightTexture; }
    RegularConstantContinuousDistribution2D& getImportanceMap() { return envLightImportanceMap; }

    void finalize()
    {
        if (envLightTexture)
        {
            CUDADRV_CHECK (cuStreamSynchronize (ctx->cuStr));
            cuTexObjectDestroy (envLightTexture);
            envLightTexture = 0;
        }

        envLightArray.finalize();
    }

 private:
    RenderContextPtr ctx = nullptr;

    cudau::Array envLightArray;
    CUtexObject envLightTexture = 0;
    RegularConstantContinuousDistribution2D envLightImportanceMap;

}; // end class SkyDomeHandler
