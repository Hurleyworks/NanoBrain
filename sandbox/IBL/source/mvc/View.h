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
