#include "Jahley.h"

const std::string APP_NAME = "HelloWorld";

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

            std::string rootFolder = getRepositoryPath (APP_NAME);
            LOG (DBUG) << rootFolder;

            LOG (DBUG) << "Hello World!";
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
