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

#include <nanogui/screen.h>

using OpenglWindowHandle = std::unique_ptr<class OpenglRenderer>;

struct GLFWwindow;

// This C++ class named "OpenglRenderer" provides a simple interface for managing an OpenGL window.
// The class includes the header file "OpenglRenderer.h" and two other header files : "berserkpch.h" and <GLFW / glfw3.h>.

// The class has a constructor that initializes the private member variables window and screen.
// It does this by iterating through a global map called __nanogui_screens, which contains a list
// of all the screens in the application.The window and screen pointers are set to the first entry 
// in this map.It is important to note that this class assumes that there is only one screen in the application.

// The class has three public methods : isOpen(), wait(), and refresh().isOpen()
// checks if the window should be closed, wait() blocks the current thread until an event occurs,
// and refresh() redraws the screen and posts an empty event to the event queue.

// This class relies on the GLFW library to create and manage the OpenGL window.
// Overall, this class provides a simple interface for managing an OpenGL window, 
// but it may not be suitable for more complex applications with multiple screens or windows.

class OpenglRenderer
{
public:
	static OpenglWindowHandle create() { return std::make_unique<OpenglRenderer>(); }

public:
	OpenglRenderer();
	~OpenglRenderer() = default;

	// Returns the nanogui screen object
	nanogui::Screen* getScreen() { return screen; }

	// Returns whether the window is open or not
	bool isOpen();

	// Renders the window by drawing all the elements on the screen
	void render() { screen->draw_all(); }

	// Calls glfwWaitEvents()
	void wait();

	// Refreshes the window
	void refresh();

private:
	nanogui::Screen* screen = nullptr;
	GLFWwindow* window = nullptr;

}; // end class OpenglRenderer
