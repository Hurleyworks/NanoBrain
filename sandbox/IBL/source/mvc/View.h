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
    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include "../physics/PhysicsUtilities.h"
#include "RenderCanvas.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "../scene/RenderableNode.h"

using namespace nanogui;

// Signals/Slots is a design pattern often used in event-driven programming. Here's the gist:
// Signal: Emitted when a specific event occurs.
// Slot: A function or method that responds to the signal.
// When a signal is emitted, all the connected slots get called.
// It's like an advanced callback system. Qt framework, for C++, popularized this pattern.

using OnDropSignal = Nano::Signal<void (const std::vector<std::string>&)>;
using OnPhyicsEngineChangeSignal = Nano::Signal<void (PhysicsEngineState)>;

class View : public nanogui::Screen, public Observer
{
 public:
    OnDropSignal dropEmitter;
    OnPhyicsEngineChangeSignal physicsStateEmitter;

 public:
    View (const DesktopWindowSettings& settings);
    ~View();

    void initialize();

    bool mouse_button_event (const nanogui::Vector2i& p, int button, bool down, int modifiers) override;
    bool mouse_motion_event (const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) override;
    bool drop_event (const std::vector<std::string>& filenames) override;
    bool keyboard_event (int key, int scancode, int action, int modifiers) override;

    void draw_contents() override;

    void setPhysicsEngineState (PhysicsEngineState state) { engineState = state; }

    RenderCanvas* getCanvas() { return canvas; }

 private:
    RenderCanvas* canvas = nullptr;
    Widget* view3d;
    Widget* toolContainer = nullptr;
    PhysicsEngineState engineState = PhysicsEngineState::Paused;

    nanogui::ref<nanogui::Button> playButton, resetButton;
    void createPlaybackPanel();

    // label colors
    int r1 = 255;
    int g1 = 100;
    int b1 = 0;
    int a1 = 255;
};
