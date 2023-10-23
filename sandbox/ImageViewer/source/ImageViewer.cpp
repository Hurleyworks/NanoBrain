#include "ImageViewer.h"

ImageViewer::ImageViewer (const DesktopWindowSettings& settings) :
    nanogui::Screen (Vector2i (settings.width, settings.height), settings.name, settings.resizable)
{
    inc_ref();

    m_background = Color{0.23f, 1.0f};
}

ImageViewer::~ImageViewer()
{
}

void ImageViewer::initialize()
{
    view3d = new Widget{this};
    view3d->set_layout (new BoxLayout{Orientation::Vertical, Alignment::Fill});

    canvas = new RenderCanvas (view3d);
    canvas->set_background_color (Color (100, 255));

    int h = DEFAULT_GUI_HEADER_HEIGHT + DEFAULT_GUI_FOOTER_HEIGHT;

    Vector2i size;
    size.x() = m_size.x();
    size.y() = m_size.y();

    view3d->set_fixed_size (size);
    canvas->set_fixed_size (size);
    view3d->set_position (Vector2i (0, 0));

    perform_layout();
}

bool ImageViewer::mouse_button_event (const nanogui::Vector2i& p, int button, bool down, int modifiers)
{
    return false;
}

bool ImageViewer::mouse_motion_event (const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers)
{
    return false;
}

bool ImageViewer::drop_event (const std::vector<std::string>& filenames)
{
    return false;
}

bool ImageViewer::keyboard_event (int key, int scancode, int action, int modifiers)
{
    return false;
}

void ImageViewer::draw_contents()
{
    clear();
    perform_layout();
}
