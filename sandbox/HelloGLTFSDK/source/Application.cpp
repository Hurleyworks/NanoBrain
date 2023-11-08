#include "Jahley.h"

const std::string APP_NAME = "HelloGLTFSDK";

#include "ReadGltf.h"

class Application : public Jahley::App
{
 public:
    Application() :
        Jahley::App()
    {
        try
        {
            // get the common content folder
            std::filesystem::path contentFolder (getCommonContentFolder());
            std::filesystem::path cube = contentFolder / "Cube/glTF/Cube.gltf";

            ReadGltf reader;
            reader.read (cube);
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
