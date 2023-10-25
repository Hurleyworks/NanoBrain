
#pragma once

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

    CUtexObject getSkyDomeTexture() { return skydomeTexture; }
    void finalize()
    {
        if (skydomeTexture)
        {
            CUDADRV_CHECK (cuStreamSynchronize (ctx->cuStr));
            cuTexObjectDestroy (skydomeTexture);
            skydomeTexture = 0;
        }

        skydomePixels.finalize();
    }

 private:
    RenderContextPtr ctx = nullptr;

    cudau::Array skydomePixels;
    CUtexObject skydomeTexture = 0;

}; // end class SkyDomeHandler
