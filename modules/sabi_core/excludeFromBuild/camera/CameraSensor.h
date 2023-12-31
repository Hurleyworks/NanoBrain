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
// This header file was auto-generated by ClassMate++
// Created: 14 Aug 2021 3:45:48 pm
// Copyright (c) 2021, HurleyWorks

#pragma once

class CameraSensor
{
 public:
    CameraSensor()
    {
        setPixelResolution (DEFAULT_DESKTOP_WINDOW_WIDTH, DEFAULT_DESKTOP_WINDOW_HEIGHT);
       // sensorIsDirty = false;
    }

    ~CameraSensor()
    {
        image.reset();
    }

    void setSensorDimensions (const Eigen::Vector2f& size)
    {
        sensorSize = size;
    }

    bool pixelResolutionHasChanged() { return sensorIsDirty; }
    void setPixelResolutionHasChanged (bool state) { sensorIsDirty = state; }

    void setPixelResolution (uint32_t newWidth, uint32_t newHeight)
    {
        assert (newWidth >= DEFAULT_MIN_WINDOW && newHeight >= DEFAULT_MIN_WINDOW);

        OIIO::ImageSpec spec;
        spec.width = newWidth;
        spec.height = newHeight;
        spec.nchannels = 4;
        spec.format = OIIO::TypeDesc::FLOAT;

        // initialize to black pixels
        image.reset (spec, OIIO::InitializePixels::Yes);

        // aspect is width / height
        aspect = (float)spec.width / (float)spec.height;

        invPixelResolution = Eigen::Vector2f (1.0f / (float)spec.width, 1.0f / (float)spec.height);

        sensorIsDirty = true;
    }

    Eigen::Vector2i getPixelResolution() const
    {
        return Eigen::Vector2i (image.spec().width, image.spec().height);
    }

    // the size of 1 pixel =  1 / pixel resolution
    const Eigen::Vector2f& pixelSize() const
    {
        return invPixelResolution;
    }

    float getPixelAspectRatio() const { return aspect; }
    const OIIO::ImageSpec& getSpec() const { return image.spec(); }

    OIIO::ImageBuf& getImage() { return image; }
    const OIIO::ImageBuf& getImage() const { return image; }

    const Eigen::Vector2f& getSensorSize() const
    {
        return sensorSize;
    }

 private:
    OIIO::ImageBuf image;
    float aspect = DEFAULT_ASPECT;
    Eigen::Vector2f invPixelResolution = Eigen::Vector2f::Ones();
    bool sensorIsDirty = false;

    // 3:2 on 35 mm:  0.036 x 0.024
    Eigen::Vector2f sensorSize = Eigen::Vector2f (0.036f, 0.024f);
};
