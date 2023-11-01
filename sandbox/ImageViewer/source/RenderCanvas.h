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
/*
    nanogui/imageview.h -- Widget used to display images.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once
#include <mace_core/mace_core.h>
#include <date.h>
#include <nanovg.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
#include <nanogui/button.h>
#include <nanogui/canvas.h>
#include <nanogui/Label.h>
#include <nanogui/icons.h>
#include <nanogui/TextBox.h>
#include <nanogui/shader.h>
#include <nanogui/popupbutton.h>
#include <nanogui/imagepanel.h>
#include <nanogui/textbox.h>

#include <GLFW/glfw3.h>

using nanogui::Button;
using nanogui::FloatBox;
using nanogui::ImagePanel;
using nanogui::IntBox;
using nanogui::Label;
using nanogui::ref;
using nanogui::Shader;
using nanogui::TextBox;
using nanogui::Vector2f;
using nanogui::Vector2i;

using nanogui::Canvas;
using nanogui::Color;
using nanogui::Shader;
using nanogui::Texture;
using nanogui::Vector2f;
using nanogui::Vector2i;

using mace::InputEvent;
using mace::MouseMode;

using Observer = Nano::Observer<>;

// signal emitters
using InputSignal = Nano::Signal<void (const mace::InputEvent& e)>;

// this is modified version of nanogui::ImageView;
class RenderCanvas : public Canvas
{
 public:
    // signals
    InputSignal inputEmitter;

 public:
    RenderCanvas (Widget* parent);

    void updateRender (const OIIO::ImageBuf& render, bool needsNewTexture = false)
    {
        if (imageTexture && !needsNewTexture)
        {
            imageTexture->upload ((uint8_t*)render.localpixels());
            set_image (imageTexture);
        }
        else if (needsNewTexture)
        {
            const OIIO::ImageSpec& spec = render.spec();
            size.x() = spec.width;
            size.y() = spec.height;
            OIIO::TypeDesc type = spec.format;

            LOG (DBUG) << "New screen size: " << spec.width << " x " << spec.height;

            imageTexture = new Texture (
                // Texture::PixelFormat::RGBA,
                // Texture::ComponentFormat::Float32,
                spec.nchannels == 3 ? Texture::PixelFormat::RGB : Texture::PixelFormat::RGBA,
                type == OIIO::TypeDesc::UINT8 ? Texture::ComponentFormat::UInt8 : Texture::ComponentFormat::Float32,
                size,
                Texture::InterpolationMode::Nearest,
                Texture::InterpolationMode::Nearest);

            imageTexture->upload ((uint8_t*)render.localpixels());
            set_image (imageTexture);
        }
    }

    /// Set the currently active image
    void set_image (Texture* image);

    bool mouse_button_event (const Vector2i& p, int button, bool down, int modifiers) override;
    bool keyboard_event (int key, int scancode, int action, int modifiers) override;
    bool mouse_drag_event (const Vector2i& p, const Vector2i& rel, int button, int modifiers) override;
    bool scroll_event (const Vector2i& p, const Vector2f& rel) override;

    void draw (NVGcontext* ctx) override;
    void draw_contents() override;

    void setMouseMode (MouseMode mode) { mouseMode = mode; }
    MouseMode getMouseMode() const { return mouseMode; }
    void grabFrame()
    {
        captureFrame = true;
    }

 protected:
    nanogui::ref<Shader> imageShader;
    nanogui::ref<Texture> imageTexture;

    MouseMode mouseMode = MouseMode::Rotate;
    Vector2i mousePressCoords = Vector2i (-1, -1);

    float imageScale = 0;
    Vector2f offset = 0;
    Vector2i size;

    Color backgroundColor;
    bool captureFrame = false;

    InputEvent::MouseButton buttonPressed = InputEvent::MouseButton::Left;

    void captureAndSaveFrame()
    {
    }
};
