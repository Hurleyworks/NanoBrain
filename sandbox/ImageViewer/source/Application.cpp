#include "Jahley.h"
#include "ImageViewer.h"

const std::string APP_NAME = "ImageViewer";

using nanogui::Vector2i;

class Application : public Jahley::App
{
 public:
    Application (DesktopWindowSettings settings = DesktopWindowSettings(), bool windowApp = false) :
        Jahley::App (settings, windowApp)
    {
        gui = dynamic_cast<ImageViewer*> (window->getScreen());
    }

    // This is the destructor of the 'Application' class.
    ~Application()
    {
    }

    // This is a public method of the 'Application' class, which overrides the virtual method 'onInit()' of the parent class 'Jahley::App'.
    void onInit() override
    {
        // get the resource folder for this project
        std::string resourceFolder = getResourcePath (APP_NAME);
        LOG (DBUG) << resourceFolder;

        gui->initialize();

        OIIO::ImageBuf img (resourceFolder + "/cubism_boston_terriers2.jpg");

        render = img;
    }

    // This is a public method of the 'Application' class, which overrides the virtual method 'update()' of the parent class 'Jahley::App'.
    void update() override
    {
        if (!renderComplete)
            gui->getCanvas()->updateRender (render, true);

        renderComplete = true;
    }

    // This is a public method of the 'Application' class, which overrides the virtual method 'onCrash()' of the parent class 'Jahley::App'.
    void onCrash() override
    {
    }

 private:
    ImageViewer* gui = nullptr;
    OIIO::ImageBuf render;
    bool renderComplete = false;
};

Jahley::App* Jahley::CreateApplication()
{
    nanogui::init(); // Initialize the nanogui library

    DesktopWindowSettings settings{};
    settings.name = APP_NAME;

    nanogui::ref<nanogui::Screen> screen = new ImageViewer (settings);
    screen->set_visible (true);

    return new Application (settings, true);
}
