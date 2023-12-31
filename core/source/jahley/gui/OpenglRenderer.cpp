// This source file was auto-generated by ClassMate++
// Created: 30 Sep 2021 7:59:04 am
// Copyright (c) 2021, HurleyWorks

#include "berserkpch.h"
#include <GLFW/glfw3.h>
#include "OpenglRenderer.h"

// this resides in nanogui::Screen.cpp and a new entry is added
// whenever a new screeen is created
extern std::map<GLFWwindow*, nanogui::Screen*> __nanogui_screens;

// ctor
OpenglRenderer::OpenglRenderer()
{
    // there can be only one screen for now
    assert (__nanogui_screens.size() == 1);

    for (const auto& it : __nanogui_screens)
    {
        window = it.first;
        screen = it.second;
    }
}

bool OpenglRenderer::isOpen()
{
    return glfwWindowShouldClose (window) != 1;
}

void OpenglRenderer::wait()
{
    glfwWaitEvents();
}

void OpenglRenderer::refresh()
{
    screen->redraw();
    glfwPostEmptyEvent();
}
