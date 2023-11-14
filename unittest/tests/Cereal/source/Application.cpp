#include "Jahley.h"

#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>

const std::string APP_NAME = "Cereal";

#ifdef CHECK
#undef CHECK
#endif

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <json/json.hpp>
using nlohmann::json;

using sabi::CameraBody;

// Define a simple struct for testing
struct MyStruct
{
    int x;
    float y;
    // Eigen::Vector3f eye;
    template <class Archive>
    void serialize (Archive& ar)
    {
        ar (CEREAL_NVP (x), CEREAL_NVP (y));
    }
};

void saveToFile (const MyStruct& data, const std::string& filename)
{
    std::ofstream os (filename);
    cereal::JSONOutputArchive archive (os);
    archive (data);
}

void loadFromFile (MyStruct& data, const std::string& filename)
{
    std::ifstream is (filename);
    cereal::JSONInputArchive archive (is);
    archive (data);
}

TEST_CASE ("Testing serialization and deserialization of MyStruct")
{
    // Original data
    MyStruct originalData{10, 3.14f};

    // Serialize
    std::stringstream ss;
    {
        cereal::JSONOutputArchive outputArchive (ss);
        outputArchive (originalData);
    }

    // Deserialize
    MyStruct deserializedData;
    {
        cereal::JSONInputArchive inputArchive (ss);
        inputArchive (deserializedData);
    }

    // Test the values
    CHECK (deserializedData.x == originalData.x);
    CHECK (deserializedData.y == doctest::Approx (originalData.y));
}

TEST_CASE ("Testing save and load to/from disk with Cereal")
{
    MyStruct originalData{20, 5.25f};
    const std::string filename = "mystruct.json";

    // Save to disk
    saveToFile (originalData, filename);

    // Load from disk
    MyStruct loadedData;
    loadFromFile (loadedData, filename);

    // Test the values
    CHECK (loadedData.x == originalData.x);
    CHECK (loadedData.y == doctest::Approx (originalData.y));
}

// Function to save CameraBody to a JSON file
void saveCameraToFile (const CameraBody& camera, const std::string& filename)
{
    std::ofstream os (filename);
    cereal::JSONOutputArchive archive (os);

    // Use CEREAL_NVP or cereal::make_nvp to give a meaningful name
    archive (cereal::make_nvp ("Camera settings", camera));
}

// Function to load CameraBody from a JSON file
void loadCameraFromFile (CameraBody& camera, const std::string& filename)
{
    std::ifstream is (filename);
    cereal::JSONInputArchive archive (is);
    archive (camera);
}

TEST_CASE ("CameraBody Serialization and Deserialization")
{
    CameraBody originalCamera;
    // Set properties of originalCamera as needed

    const std::string filename = "cameraBody.json";

    // Save to disk
    saveCameraToFile (originalCamera, filename);

    // Load from disk
    CameraBody loadedCamera;
    loadCameraFromFile (loadedCamera, filename);

    // Compare properties of originalCamera and loadedCamera
    CHECK (loadedCamera.getEyePoint() == originalCamera.getEyePoint());
    CHECK (loadedCamera.getTarget() == originalCamera.getTarget());
    CHECK (loadedCamera.getApeture() == originalCamera.getApeture());
    CHECK (loadedCamera.getFocalLength() == originalCamera.getFocalLength());
    // Add more checks for other properties as needed
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
