#include "Jahley.h"
#include "mvc/View.h"
#include "mvc/Controller.h"
#include "mvc/Model.h"

const std::string APP_NAME = "NanoBrain1";

using Eigen::Vector3f;
using nanogui::Vector2i;
using sabi::CameraBody;

class Application : public Jahley::App
{
 public:
    Application (DesktopWindowSettings settings = DesktopWindowSettings(), bool windowApp = false) :
        Jahley::App (settings, windowApp) // Call the constructor of the parent class 'Jahley::App' and pass the arguments.
    {
        view = dynamic_cast<View*> (window->getScreen());

        // create the default camera
        camera = std::make_shared<CameraBody>();
        camera->setFocalLength (0.055f); // 55 mm lens
        camera->lookAt (Eigen::Vector3f (0.0f, 0.0f, 3.5f), Eigen::Vector3f (0.0f, 0.0f, 0.0f), Eigen::Vector3f (0.0f, 1.0f, 0.0f));
    }

    ~Application()
    {
        nanogui::shutdown();
    }

    void onInit() override
    {
        view->initialize();
        controller.initialize();

        std::string resourceFolder = getResourcePath (APP_NAME);
        std::string repoFolder = getRepositoryPath (APP_NAME);

        model.init (camera, resourceFolder, repoFolder);

        // connect signals/slots
        view->dropEmitter.connect<&Model::onDrop> (model);
        view->physicsStateEmitter.connect<&Model::setPhysicsEngineSate> (model);
        model.physicsStateEmitter.connect<&View::setPhysicsEngineState> (*view);
        view->getCanvas()->inputEmitter.connect<&App::onInputEvent> (*this);
    }

    void update() override
    {
        // update physics before rendering
        model.updatePhysics();
        model.render();

        // get the render
        const OIIO::ImageBuf& img = camera->getSensorPixels();

        bool needsNewRenderTexture = (img.spec().width != lastImageWidth ||
                                      img.spec().height != lastImageHeight ||
                                      img.spec().nchannels != lastChannelCount);

        view->getCanvas()->updateRender (std::move (camera->getSensorPixels()), needsNewRenderTexture);

        lastImageWidth = img.spec().width;
        lastImageHeight = img.spec().height;
        lastChannelCount = img.spec().nchannels;
    }

    void onInputEvent (const mace::InputEvent& e) override
    {
        // no need to process moves is there?
        if (e.getType() == InputEvent::Type::Move) return;

        controller.onInputEvent (e, camera);
    }

    void onCrash() override
    {
    }

 private:
    View* view = nullptr;
    Model model;
    Controller controller;

    CameraHandle camera = nullptr;
    bool sensorDirty = true;

    uint32_t lastImageWidth = 0;
    uint32_t lastImageHeight = 0;
    uint32_t lastChannelCount = 0;
};

Jahley::App* Jahley::CreateApplication()
{
    nanogui::init();

    DesktopWindowSettings settings{};
    settings.name = APP_NAME;

    nanogui::ref<nanogui::Screen> screen = new View (settings);
    screen->set_visible (true);

    return new Application (settings, true);
}
