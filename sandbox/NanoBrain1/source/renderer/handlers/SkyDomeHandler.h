
#pragma once

// much taken from Shocker GfxExp
// https://github.com/shocker-0x15/GfxExp

#include "../common_host.h"
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
