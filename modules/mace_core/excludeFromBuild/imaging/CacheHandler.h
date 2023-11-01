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

using namespace OIIO;

using ImageCacheHandlerRef = std::shared_ptr<class ImageCacheHandler>;

class ImageCacheHandler
{
 public:
    static ImageCacheHandlerRef create() { return std::make_shared<ImageCacheHandler>(); }
    using CachedImageSet = std::set<std::string>;

 public:
    ImageCacheHandler()
    {
        // Create an image imageCache and set some options
        imageCache = ImageCache::create();
        imageCache->attribute ("max_memory_MB", 500.0f);
        imageCache->attribute ("autotile", 64);

        threadPool = OIIO::default_thread_pool();
    }
    ~ImageCacheHandler()
    {
        info();
        imagePathSet.clear();
        ImageCache::destroy (imageCache);
    }

    void info()
    {
        LOG (DBUG) << imageCache->getstats();
    }

    size_t getCachedImageCount() { return imagePathSet.size(); }

    ImageBuf getNextImage()
    {
        size_t imageCount = imagePathSet.size();

        // our path set must match the number of files in the cache
        int total_cached;
        imageCache->getattribute ("total_files", total_cached);
        if (imageCount == 0 || total_cached != imageCount) return ImageBuf();

        // maybe reset the image index so we don't go out of bounds
        if (imageIndex >= imageCount) imageIndex = 0;

        // https://thispointer.com/how-to-access-element-by-index-in-a-set-c/
        std::set<std::string>::iterator it = std::next (imagePathSet.begin(), imageIndex++);

        return getCachedImage (*it);
    }

    ImageBuf getCachedImage (const std::string& imagePath, bool fitToScreen = true)
    {
        pixels.clear();
        floatPixels.clear();

        ustring filename (imagePath);
        ImageSpec spec;
        imageCache->get_imagespec (filename, spec);

        if (spec.format == OIIO::TypeDesc::UINT8)
        {
            pixels.resize (spec.width * spec.height * spec.nchannels * sizeof (uint8_t));
            imageCache->get_pixels (filename, 0, 0, 0, spec.width, 0, spec.height,
                                    0, 1, spec.format, pixels.data());

            if (fitToScreen)
            {
                ImageBuf image (spec, pixels.data());
                // const OIIO::ImageSpec& spec = image.spec();
                int w = spec.width;
                int h = spec.height;
                int nChan = spec.nchannels;

                float aspectRatio = (float)spec.width / (float)spec.height;
                float resizedWidth = DEFAULT_DESKTOP_WINDOW_WIDTH;
                float resizedHeight = resizedWidth / aspectRatio;

                OIIO::ROI roi (0, (int)resizedWidth, 0, (int)resizedHeight, 0, 1, /*chans:*/ 0, spec.nchannels);
                OIIO::ImageBuf img = OIIO::ImageBufAlgo::resize (image, "", 0, roi);

                return OIIO::ImageBufAlgo::resize (image, "", 0, roi);
            }
            else
                return ImageBuf (spec, pixels.data());
        }
        else if (spec.format == OIIO::TypeDesc::FLOAT)
        {
            floatPixels.resize (spec.width * spec.height * spec.nchannels * sizeof (float));
            imageCache->get_pixels (filename, 0, 0, 0, spec.width, 0, spec.height,
                                    0, 1, spec.format, floatPixels.data());

            return ImageBuf (spec, floatPixels.data());
        }

        return ImageBuf();
    }

    void addImage (const std::string& imagePath)
    {
        imagePathSet.insert (imagePath);

        threadPool->push (&ImageCacheHandler::addImageToCache, imagePath);
    }

    void addImageFolderToCache (const std::string& imageFolder)
    {
        std::filesystem::path folder (imageFolder);

        if (!std::filesystem::is_directory (folder))
            throw std::runtime_error ("Invalid folder: " + imageFolder);

        std::vector<std::string> files = FileServices::getFiles (imageFolder, supportedImageFormats(), true);

        for (const auto& path : files)
        {
            std::filesystem::path f (path);
            if (std::filesystem::is_directory (f)) continue;

            imagePathSet.insert (path);
            threadPool->push (&ImageCacheHandler::addImageToCache, path);
        }
    }

    void addImagePathsToCache (const std::vector<std::string>& paths)
    {
        for (const auto& path : paths)
        {
            // Pass on directories
            std::filesystem::path f (path);
            if (std::filesystem::is_directory (f)) continue;

            imagePathSet.insert (path);
            threadPool->push (&ImageCacheHandler::addImageToCache, path);
        }
    }


    static void addImageToCache (int thread_id, const std::string& imagePath)
    {
        LOG (DBUG) << imagePath;
        try
        {
            ImageBuf iBuf (imagePath, 0, 0, imageCache);
            if (iBuf.read())
            {
                if (iBuf.cachedpixels())
                {
                }
                else
                {
                    LOG (CRITICAL) << "Pixels not cached " << imagePath;
                }
            }
            else
            {
                LOG (CRITICAL) << "File read failed!"
                               << "::" << imagePath;
            }
        }
        catch (std::exception& e)
        {
            LOG (CRITICAL) << e.what();
        }
    }

 private:
    static ImageCache* imageCache;
    CachedImageSet imagePathSet;
    uint32_t imageIndex = 0;
    OIIO::thread_pool* threadPool = nullptr;

    // pixels must persist until reaching opengl renderer
    // FIXME
    std::vector<uint8_t> pixels;
    std::vector<float> floatPixels;
};