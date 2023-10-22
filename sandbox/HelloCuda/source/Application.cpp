#include "Jahley.h"

// from https://github.com/theComputeKid/premake5-cuda
const std::string APP_NAME = "HelloCuda";

int runKernel(); 

class Application : public Jahley::App
{
 public:
    Application() :
        Jahley::App()
    {
        try
        {
            LOG (DBUG) << "Hello from the CPU!";
            runKernel();
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
