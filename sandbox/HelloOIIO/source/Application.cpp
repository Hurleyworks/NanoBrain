#include "Jahley.h"

const std::string APP_NAME = "HelloOIIO";

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

            OIIO::ImageBuf image (resourceFolder + std::string ("/broccoli_dragon.jpg"));
            const OIIO::ImageSpec& spec = image.spec();
            int w = spec.width;
            int h = spec.height;
            int nChan = spec.nchannels;

            LOG (DBUG) << w << ", " << h << ", " << nChan;
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
