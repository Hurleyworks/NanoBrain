// This source file was auto-generated by ClassMate++
// Created: 26 Mar 2022 9:08:29 am
// Copyright (c) 2022, HurleyWorks

#include "Controller.h"

const float moveFactor = 0.0075f;

void Controller::onInputEvent (const InputEvent& input, CameraHandle& camera)
{
    mouseCoords = Eigen::Vector2f (input.getX(), input.getY());
    MouseMode mouseMode = input.getMouseMode();

    // record mouse movement
    float deltaX = input.getScreenMovementX();
    float deltaY = input.getScreenMovementY();

    float zoomFactor = DEFAULT_ZOOM_FACTOR;

    uint32_t keyModifers = input.getKeyboardModifiers();

    // shift key to increase Zoom factor
    if (input.getKeyboardModifiers() & static_cast<int> (InputEvent::Modifier::Shift))
    {
        zoomFactor *= DEFAULT_ZOOM_MULTIPLIER;
    }

    switch (input.getType())
    {
        case InputEvent::Type::Press:
        {
            if (input.getButton() == InputEvent::MouseButton::Left && mouseMode == MouseMode::Rotate)
            {
                camera->startTracking();
                camera->setDirty (true);

                // have to store the button pressed because
                // getButton doesn't give the right answer in Drag events
                buttonPressed = InputEvent::MouseButton::Left;
            }
            else if (input.getButton() == InputEvent::MouseButton::Left && mouseMode == MouseMode::Translate)
            {
                startMouseCoords = mouseCoords;

                // have to store the button pressed because
                // getButton doesn't give the right answer in Drag events
                buttonPressed = InputEvent::MouseButton::Left;
            }
            else if (input.getButton() == InputEvent::MouseButton::Left && mouseMode == MouseMode::Paint)
            {
                camera->setDirty (true);
            }
            else if (input.getButton() == InputEvent::MouseButton::Right && input.getKey() == 70)
            {
                camera->setDirty (true);
                buttonPressed = InputEvent::MouseButton::Right;
            }
            else if (input.getButton() == InputEvent::MouseButton::Right)
            {
                camera->setDirty (true);
                buttonPressed = InputEvent::MouseButton::Right;
                // LOG (DBUG) << "-----------------------Front end PICKING: " << input.getX() << ", " << input.getY();
            }

            break;
        }

        case InputEvent::Type::Release:
        {
            camera->setDirty (false);
            break;
        }

        case InputEvent::Type::Move:
            break;

        case InputEvent::Type::ScrollUp:
            camera->zoom (zoomFactor);
            camera->setDirty (true);
            break;

        case InputEvent::Type::ScrollDown:
            camera->zoom (zoomFactor * -1);
            camera->setDirty (true);
            break;

        case InputEvent::Type::Drag:

            // LMB for camera trackball, RMB for picking
            if (buttonPressed == InputEvent::MouseButton::Left && mouseMode == MouseMode::Rotate)
            {
                camera->track (Eigen::Vector2f (input.getX(), input.getY()));
            }
            else if (buttonPressed == InputEvent::MouseButton::Left && mouseMode == MouseMode::Translate)
            {
                // move the camera in X and Y in camera space
                Pose t = camera->getPose();
                Eigen::Vector3f eye = camera->getEyePoint();

                // move along world space when control key is pressed
                if (input.getKeyboardModifiers() == (int)InputEvent::Modifier::Ctrl)
                {
                    eye.x() += deltaX * moveFactor;
                    eye.y() += deltaY * moveFactor;
                }
                else // move in camera space
                {
                    Eigen::Vector3f camUP = t.linear().col (1).normalized();
                    Eigen::Vector3f camRight = t.linear().col (0).normalized();

                    eye += (camRight * deltaX * moveFactor + camUP * deltaY * moveFactor);
                }

                camera->setEyePoint (eye);
                // camera->lookAt (eye);
            }
            else if (input.getButton() == InputEvent::MouseButton::Left && mouseMode == MouseMode::Paint)
            {
            }

            camera->setDirty (true);
            break;

        case InputEvent::Type::KeyRepeat:
        {
            camera->setDirty (true);
            break;
        }
        case InputEvent::Type::KeyPress:
        {
            camera->setDirty (true);

            // space bar for toggle between mouse modes
            if (input.getKey() == 32)
            {
            }

            break;
        }
    }
}
