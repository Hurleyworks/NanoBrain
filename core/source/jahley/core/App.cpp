// This source file was auto-generated by ClassMate++
// Created: 26 Jan 2019 8:47:47 pm
// Copyright (c) 2019, HurleyWorks

#include "berserkpch.h"
#include "App.h"

namespace Jahley
{
    using namespace std::chrono_literals;

    App::App(DesktopWindowSettings settings, bool windowApp) :
        windowApp(windowApp),
        errorCallback(std::bind(&App::onFatalError, this, std::placeholders::_1)),
        preCrashCallback(std::bind(&App::preCrash, this)),
        log(errorCallback, preCrashCallback),
        refreshWait(settings.refreshRate)
    {
        if (windowApp)
        {
            window = OpenglRenderer::create();
        }
    }

    App::~App()
    {
    }

    void App::run()
    {
        // let the client initialize
        onInit();

        while (window->isOpen())
        {
            // let the client do their thinng
            update();

            window->render();
            window->wait();

            // post an empty event to GLFW after waiting
            // for user defined amount of time
            std::this_thread::sleep_for(refreshWait);

            window->refresh();
        }

        nanogui::shutdown();
    }

    // preCrash
    void App::preCrash()
    {
        // let the client throw up a warning if it can
        onCrash();

#ifndef NDEBUG
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
        __debugbreak();
#endif
#endif
    }

    // onFatalError
    void App::onFatalError(g3::FatalMessagePtr fatal_message)
    {
        LOG(CRITICAL) << fatal_message.get()->toString();

        g3::internal::pushFatalMessageToLogger(fatal_message);
    }
} // namespace Jahley