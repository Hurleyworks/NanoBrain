#include "View.h"

View::View (const DesktopWindowSettings& settings) :
    nanogui::Screen (Vector2i (settings.width, settings.height), settings.name, settings.resizable)
{
    inc_ref(); // Huh?

    m_background = Color{0.23f, 1.0f};
}

View::~View()
{
}

void View::initialize()
{
    view3d = new Widget{this};
    view3d->set_layout (new BoxLayout{Orientation::Vertical, Alignment::Fill});

    bool doPostProcessing = true;
    canvas = new RenderCanvas (view3d, doPostProcessing);
    canvas->set_background_color (Color (100, 255));

    int h = DEFAULT_GUI_HEADER_HEIGHT + DEFAULT_GUI_FOOTER_HEIGHT;

    Vector2i size;
    size.x() = m_size.x();
    size.y() = m_size.y();

    view3d->set_fixed_size (size);
    canvas->set_fixed_size (size);
    view3d->set_position (Vector2i (0, 0));

    toolContainer = new Widget{this};
    toolContainer->set_layout (new GridLayout{Orientation::Horizontal, 6, Alignment::Fill, 0, 10});

    toolContainer->set_position (Vector2i (100, m_size.y() - DEFAULT_GUI_FOOTER_HEIGHT));
   
    createPlaybackPanel();
}

void View::createPlaybackPanel()
{
    auto sim_label = new Label{toolContainer, "Play ", "sans-bold"};
    sim_label->set_color (Color (r1, g1, b1, a1));

    auto playback = new Widget{toolContainer};
    playback->set_layout (new GridLayout{Orientation::Horizontal, 4, Alignment::Fill, 5, 2});

    playButton = new Button (playback, "", FA_PLAY);
    playButton->set_text_color (nanogui::Color (50, 255, 100, 150));
    playButton->set_enabled (true);
    playButton->set_callback ([this]()
                              {
             switch (engineState)
                                  {
                                      case PhysicsEngineState::Paused:
                                          playButton->set_icon (FA_PAUSE);
                                          playButton->set_text_color (nanogui::Color (255, 100, 100, 150));
                                          resetButton->set_enabled (true);
                                          engineState = PhysicsEngineState (PhysicsEngineState::Running);
                                          physicsStateEmitter.fire (engineState);
                                          break;

                                      case PhysicsEngineState::Running:
                                          playButton->set_icon (FA_PLAY);
                                          playButton->set_text_color (nanogui::Color (100, 255, 100, 150));
                                          engineState = PhysicsEngineState (PhysicsEngineState::Paused);
                                          physicsStateEmitter.fire (engineState);
                                          break;
                                      default:
                                          break;

                                  } });
    playButton->set_tooltip ("Run");

    resetButton = new Button (playback, "", FA_SYNC_ALT);
    resetButton->set_callback ([this]()
                               {
                                   playButton->set_icon (FA_PLAY);
                                   playButton->set_text_color (nanogui::Color (100, 255, 100, 150));
                                   playButton->set_enabled (true);
                                   resetButton->set_enabled (false);
                                   engineState = PhysicsEngineState (PhysicsEngineState::Reset);
                                   physicsStateEmitter.fire (engineState); });
    resetButton->set_tooltip ("Reset");
    resetButton->set_enabled (false);
}

bool View::mouse_button_event (const nanogui::Vector2i& p, int button, bool down, int modifiers)
{
    // must send back to Screen!
    Screen::mouse_button_event (p, button, down, modifiers);

    return false;
}

bool View::mouse_motion_event (const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers)
{
    return false;
}

bool View::drop_event (const std::vector<std::string>& filenames)
{
    dropEmitter.fire (filenames);
    return false;
}

bool View::keyboard_event (int key, int scancode, int action, int modifiers)
{
    return false;
}

void View::draw_contents()
{
    clear();
    perform_layout();
}
