#include "Jahley.h"
#include "View.h"

const std::string APP_NAME = "HelloNanogui";

using nanogui::Vector2i;

// This is a C++ class named 'Application' which inherits from the 'Jahley::App' class.
// It has public, private methods and a private member variable named 'gui'.

class Application : public Jahley::App
{
 public:
    // This is the constructor of the 'Application' class, which takes an optional argument of 'DesktopWindowSettings' type
    // named 'settings', and a boolean argument named 'windowApp' which is also optional.
    Application (DesktopWindowSettings settings = DesktopWindowSettings(), bool windowApp = false) :
        Jahley::App (settings, windowApp) // Call the constructor of the parent class 'Jahley::App' and pass the arguments.
    {
        // get the resource folder for this project
        std::string resourceFolder = getResourcePath (APP_NAME);
       
        LOG (DBUG) << resourceFolder;

        std::string rootFolder = getRepositoryPath (APP_NAME);
        LOG (DBUG) << rootFolder;

        // Initialize the member variable 'gui' by casting the pointer to the object of 'View' type.
        // This pointer is obtained by calling the 'getScreen()' method of the 'window' object which is a member of the parent class 'Jahley::App'.
        gui = dynamic_cast<View*> (window->getScreen());
    }

    // This is the destructor of the 'Application' class.
    ~Application()
    {
        // This destructor is empty, as no memory has been allocated dynamically in the class.
    }

    // This is a public method of the 'Application' class, which overrides the virtual method 'onInit()' of the parent class 'Jahley::App'.
    void onInit() override
    {
        // This method is empty, it can be overridden by a derived class to perform some initialization tasks.
    }

    // This is a public method of the 'Application' class, which overrides the virtual method 'update()' of the parent class 'Jahley::App'.
    void update() override
    {
        // This method is empty, it can be overridden by a derived class to update the application state.
    }

    // This is a public method of the 'Application' class, which overrides the virtual method 'onCrash()' of the parent class 'Jahley::App'.
    void onCrash() override
    {
        // This method is empty, it can be overridden by a derived class to handle any unhandled exceptions that caused the application to crash.
    }

 private:
    View* gui = nullptr;
};

// This function creates an instance of the Application class defined in the Jahley namespace.
// It first initializes the nanogui library using nanogui::init() function.
// Then, it creates a DesktopWindowSettings object named 'settings' and sets its name to APP_NAME.
// Next, it creates an instance of the View class using the 'settings' object and stores it in a nanogui::ref pointer named 'screen'.
// After that, it sets the 'visible' property of the 'screen' instance to true.
// Finally, it creates an instance of the Application class using the 'settings' object and returns it.
Jahley::App* Jahley::CreateApplication()
{
    nanogui::init(); // Initialize the nanogui library

    // Create a DesktopWindowSettings object and set its name to APP_NAME
    DesktopWindowSettings settings{};
    settings.name = APP_NAME;

    // Create a View instance using the 'settings' object and store it in a nanogui::ref pointer named 'screen'
    nanogui::ref<nanogui::Screen> screen = new View (settings);
    screen->set_visible (true); // Set the 'visible' property of the 'screen' instance to true

    // Create an instance of the Application class using the 'settings' object and return it
    return new Application (settings, true);
}

