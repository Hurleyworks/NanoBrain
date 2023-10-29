#include "Jahley.h"

// shocker linalg
#include <basic_types.h>

const std::string APP_NAME = "ShockerEigen";

#ifdef CHECK
#undef CHECK
#endif

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <json/json.hpp>
using nlohmann::json;

struct PerspectiveCameraShocker
{
    float aspect = 1.61704540;
    float fovY = 0.443422198;
    Point3D position = {0.0f, 0.0f, -2.5f};
    Matrix3x3 orientation;  // default is indentity

    CUDA_COMMON_FUNCTION Point2D calcScreenPosition (const Point3D& posInWorld) const
    {
        Matrix3x3 invOri = invert (orientation);
        Point3D posInView (invOri * (posInWorld - position));
        Point2D posAtZ1 (posInView.x / posInView.z, posInView.y / posInView.z);
        float h = 2 * std::tan (fovY / 2);
        float w = aspect * h;
        return Point2D (1 - (posAtZ1.x + 0.5f * w) / w,
                        1 - (posAtZ1.y + 0.5f * h) / h);
    }
};

struct PerspectiveCameraEigen
{
    PerspectiveCameraEigen()
    {
        orientation.setIdentity();
    }
    float aspect = 1.61704540;
    float fovY = 0.443422198;
    Eigen::Vector3f position = {0.0f, 0.0f, -2.5f};
    Eigen::Matrix3f orientation;

    Eigen::Vector2f calcScreenPosition (const Eigen::Vector3f& posInWorld) const
    {
        Eigen::Matrix3f invOri = orientation.inverse();
        Eigen::Vector3f posInView = invOri * (posInWorld - position);
        Eigen::Vector2f posAtZ1 (posInView.x() / posInView.z(), posInView.y() / posInView.z());
        float h = 2 * std::tan (fovY / 2);
        float w = aspect * h;
        return Eigen::Vector2f (1 - (posAtZ1.x() + 0.5f * w) / w,
                                1 - (posAtZ1.y() + 0.5f * h) / h);
    }
};

TEST_CASE ("Testing PerspectiveCamera implementations")
{
    PerspectiveCameraShocker shockerCamera;
    PerspectiveCameraEigen eigenCamera;

    Point3D origPoint = {1.0f, 2.0f, 3.0f};
    Eigen::Vector3f eigenPoint (1.0f, 2.0f, 3.0f);

    Point2D origResult = shockerCamera.calcScreenPosition (origPoint);
    Eigen::Vector2f eigenResult = eigenCamera.calcScreenPosition (eigenPoint);

    CHECK (origResult.x == doctest::Approx (eigenResult.x()));
    CHECK (origResult.y == doctest::Approx (eigenResult.y()));
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
