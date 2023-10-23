/*
    src/example4.cpp -- C++ version of an example application that shows
    how to use the OpenGL widget. For a Python implementation, see
    '../python/example4.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "RenderCanvas.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>

using namespace nanogui;

constexpr float Pi = 3.14159f;
static const int SIDEBAR_MIN_WIDTH = 230;

class ImageViewer : public nanogui::Screen
{
 public:
    ImageViewer (const DesktopWindowSettings& settings);
    ~ImageViewer();

    void initialize();

    bool mouse_button_event (const nanogui::Vector2i& p, int button, bool down, int modifiers) override;
    bool mouse_motion_event (const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) override;

    bool drop_event (const std::vector<std::string>& filenames) override;

    bool keyboard_event (int key, int scancode, int action, int modifiers) override;

    void draw_contents() override;

    RenderCanvas* getCanvas() { return canvas; }

 private:
    RenderCanvas* canvas = nullptr;
    Widget* view3d;
};
