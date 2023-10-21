#include "Jahley.h"

const std::string APP_NAME = "OIIO";

#ifdef CHECK
#undef CHECK
#endif

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <json/json.hpp>
using nlohmann::json;

#include <OpenImageIO/imageio.h>
using namespace OIIO;

// Here is the simplest sequence required to open an image file,
// find out its resolution, and read the pixels (converting them
// into 8 - bit values in memory, even if that’s not the way
// they’re stored in the file) :
TEST_CASE ("Load image image")
{
    // get the resource folder for this project
    std::string resourceFolder = getResourcePath (APP_NAME);
    LOG (DBUG) << resourceFolder;

    // width 1600, height 1067, nchannels 3
     const std::string filename (resourceFolder + std::string ("/orb.jpg"));

    auto inp = ImageInput::open (filename);
    if (!inp)
    {
        LOG (CRITICAL) << "Could not open " << filename
                       << ", error = " << OIIO::geterror();
        return;
    }
    const ImageSpec& spec = inp->spec();
    int xres = spec.width;
    int yres = spec.height;
    int nchannels = spec.nchannels;

    CHECK (xres == 512);
    CHECK (yres == 512);
    CHECK (nchannels == 3);

    // Note that in this example, we don’t care what data format is used for
    // the pixel data in the file — we will request unsigned 8 bit integers
    // and rely on OpenImageIO’s ability to convert to our requested format
    // from the native data format of the file.
    auto pixels = std::unique_ptr<uint8_t[]> (new uint8_t[xres * yres * nchannels]);
    inp->read_image (0, 0, 0, nchannels, TypeDesc::UINT8, &pixels[0]);
    inp->close();
}

class Application : public Jahley::App
{
 public:
    Application() :
        Jahley::App()
    {
        doctest::Context().run();
    }

 private:
};

Jahley::App* Jahley::CreateApplication()
{
    return new Application();
}
