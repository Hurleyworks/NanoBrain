
#include "SkyDomeHandler.h"

// ctor
SkyDomeHandler::SkyDomeHandler (RenderContextPtr ctx) :
    ctx (ctx)
{
}

// dtor
SkyDomeHandler::~SkyDomeHandler()
{
}

void SkyDomeHandler::addSkyDomeImage (const OIIO::ImageBuf&& image)
{
    ScopedStopWatch sw ("ADDING SKYDOME");

    // FIXME
    if (image.spec().nchannels != 3)
    {
        return;
    }

    assert (image.spec().format == OIIO::TypeDesc::FLOAT);

    finalize();

    // need to add an alpha channel
    int channelorder[] = {0, 1, 2, 3};
    float channelvalues[] = {0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/, 1.0f};
    std::string channelnames[] = {"", "", "", "A"};
    OIIO::ImageBuf rgba = OIIO::ImageBufAlgo::channels (image, 4, channelorder, channelvalues, channelnames);

    const OIIO::ImageSpec& spec = rgba.spec();

    cudau::TextureSampler sampler_float;
    sampler_float.setXyFilterMode (cudau::TextureFilterMode::Linear);
    sampler_float.setWrapMode (0, cudau::TextureWrapMode::Clamp);
    sampler_float.setWrapMode (1, cudau::TextureWrapMode::Clamp);
    sampler_float.setMipMapFilterMode (cudau::TextureFilterMode::Point);
    sampler_float.setReadMode (cudau::TextureReadMode::ElementType);

    int32_t width = spec.width;
    int32_t height = spec.height;

    // don't delete ... owned by ImageBuf
    float* const textureData = static_cast<float*> (rgba.localpixels());

    for (int y = 0; y < height; ++y)
    {
        float theta = M_PI * (y + 0.5f) / height;
        float sinTheta = std::sin (theta);
        for (int x = 0; x < width; ++x)
        {
            uint32_t idx = 4 * (y * width + x);
            textureData[idx + 0] = std::max (textureData[idx + 0], 0.0f);
            textureData[idx + 1] = std::max (textureData[idx + 1], 0.0f);
            textureData[idx + 2] = std::max (textureData[idx + 2], 0.0f);
        }
    }

    skydomePixels.initialize2D (
        ctx->cuCtx, cudau::ArrayElementType::Float32, 4,
        cudau::ArraySurface::Disable, cudau::ArrayTextureGather::Disable,
        width, height, 1);
    skydomePixels.write (textureData, width * height * 4);

    skydomeTexture = sampler_float.createTextureObject (skydomePixels);
}
