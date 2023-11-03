
#pragma once

#include <sabi_core/sabi_core.h>

using mace::InputEvent;
using mace::MouseMode;
using sabi::CameraHandle;

class Controller
{
 public:
    Controller() = default;
    ~Controller() = default;

    void initialize() {}
    void onInputEvent (const InputEvent& input, CameraHandle& camera);

 private:
    Eigen::Vector2f mouseCoords;
    Eigen::Vector2f startMouseCoords;
    InputEvent previousInput;
    InputEvent::MouseButton buttonPressed = InputEvent::MouseButton::Left;

}; // end class Controller
