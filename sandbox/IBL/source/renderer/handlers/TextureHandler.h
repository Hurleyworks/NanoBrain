#pragma once

#include "../RenderContext.h"

using TextureHandlerRef = std::shared_ptr<class TextureHandler>;
using mace::ImageCacheHandlerRef;

class TextureHandler
{
 public:
    static TextureHandlerRef create (RenderContextPtr ctx) { return std::make_shared<TextureHandler> (ctx); }

 public:
    TextureHandler (RenderContextPtr ctx);
    ~TextureHandler();

    CUtexObject createCudaTextureFromImage (const std::filesystem::path& fullPath);

 private:
    RenderContextPtr ctx = nullptr;
    ImageCacheHandlerRef imageCache = nullptr;
    std::vector<std::shared_ptr<cudau::Array>> textures; 
};
