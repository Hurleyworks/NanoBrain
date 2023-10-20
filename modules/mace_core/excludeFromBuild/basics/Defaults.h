#pragma once

// You can make it constexpr if its value is known at compile - time and won't change.
// constexpr ensures that the value is computed at compile-time.
constexpr float PHI = 1.618f;
constexpr float DEFAULT_DESKTOP_WINDOW_HEIGHT = 800.0f;
constexpr float DEFAULT_DESKTOP_WINDOW_WIDTH = DEFAULT_DESKTOP_WINDOW_HEIGHT * PHI;

// std::string is not a literal type, so it can't be used with constexpr.
// constexpr requires the variable to be initialized with a constant expression,
// and std::string involves dynamic memory allocation.
const std::string DEFAULT_DESKTOP_WINDOW_NAME = "DesktopWindow";

constexpr int DEFAULT_DESKTOP_WINDOW_REFRESH_RATE = 16;
constexpr bool DEFAULT_DESKTOP_WINDOW_RESIZABLE = true;

struct DesktopWindowSettings
{
    uint32_t width = static_cast<uint32_t> (DEFAULT_DESKTOP_WINDOW_WIDTH);
    uint32_t height = static_cast<uint32_t> (DEFAULT_DESKTOP_WINDOW_HEIGHT);
    std::string name = DEFAULT_DESKTOP_WINDOW_NAME;
    int refreshRate = DEFAULT_DESKTOP_WINDOW_REFRESH_RATE;
    bool resizable = DEFAULT_DESKTOP_WINDOW_RESIZABLE;
};