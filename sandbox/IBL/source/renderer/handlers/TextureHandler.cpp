#include "TextureHandler.h"
#include "Handlers.h"

using mace::ImageCacheHandler;

TextureHandler::TextureHandler (RenderContextPtr ctx) :
    ctx (ctx)
{
    imageCache = ImageCacheHandler::create();
}

TextureHandler::~TextureHandler()
{
}

CUtexObject TextureHandler::createCudaTextureFromImage (const std::filesystem::path& fullPath)
{
    OIIO::ImageBuf image = imageCache->getCachedImage (fullPath.generic_string(), false);

    OIIO::ImageSpec spec = image.spec();
    if (spec.format != OIIO::TypeDesc::UINT8) return 0;

    OIIO::ImageBuf rgba;
    if (spec.nchannels != 4)
    {
        // need to add an alpha channel
        int channelorder[] = {0, 1, 2, 3};
        float channelvalues[] = {0 /*ignore*/, 0 /*ignore*/, 0 /*ignore*/, 1.0f};
        std::string channelnames[] = {"", "", "", "A"};
        rgba = OIIO::ImageBufAlgo::channels (image, 4, channelorder, channelvalues, channelnames);
        spec = rgba.spec();
    }
    else
        rgba = image;

    const uint8_t* const linearImageData = static_cast<uint8_t*> (rgba.localpixels());

    std::shared_ptr<cudau::Array> array = std::make_shared<cudau::Array>();
    array->initialize2D (ctx->cuCtx, cudau::ArrayElementType::UInt8, 4,
                         cudau::ArraySurface::Disable, cudau::ArrayTextureGather::Disable,
                         spec.width, spec.height, 1);
    array->write<uint8_t> (linearImageData, spec.width * spec.height * 4);

    cudau::TextureSampler texSampler;
    texSampler.setXyFilterMode (cudau::TextureFilterMode::Point);
    texSampler.setMipMapFilterMode (cudau::TextureFilterMode::Point);
    texSampler.setIndexingMode (cudau::TextureIndexingMode::NormalizedCoordinates);
    texSampler.setReadMode (cudau::TextureReadMode::NormalizedFloat_sRGB);

    textures.emplace_back (array);
    return texSampler.createTextureObject (*array);
}
