#include "Jahley.h"

#include <ndNewton.h>

const std::string APP_NAME = "HelloNewton";

class Application : public Jahley::App
{
 public:
    Application() :
        Jahley::App()
    {
        try
        {
            // get the resource folder for this project
            std::string resourceFolder = getResourcePath (APP_NAME);
            LOG (DBUG) << resourceFolder;

            LOG (DBUG) << "Hello Newton!";

            ndWorld world;
            world.SetSubSteps (2);
            world.Update (1.0f / 60.0f);
            world.Sync();
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    ~Application()
    {
    }

    void onCrash() override
    {
    }

 private:
};

Jahley::App* Jahley::CreateApplication()
{
    return new Application();
}
