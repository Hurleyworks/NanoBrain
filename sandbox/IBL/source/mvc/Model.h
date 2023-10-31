
#pragma once

#include "../renderer/nvcc/CudaCompiler.h"
#include "../renderer/Renderer.h"
#include "../physics/NewtonEngine.h"

using sabi::CameraHandle;
using OnPhyicsEngineChangeSignal = Nano::Signal<void (PhysicsEngineState)>;

class Model : public Observer
{
 public:
    OnPhyicsEngineChangeSignal physicsStateEmitter;

 public:
    Model() = default;
    ~Model() = default;

    void init (CameraHandle& camera, const std::string& resourceFolder, const std::string& repoFolder, const std::string& commonFolder);
    void render();
    void updatePhysics();
    void onDrop (const std::vector<std::string>& filenames);
    void setPhysicsEngineSate (PhysicsEngineState state) { engineState = state; }

 private:
    CudaCompiler nvcc;
    Renderer renderer;
    NewtonEngine newton;
    PhysicsEngineState engineState = PhysicsEngineState::Paused;

    void processPath (const std::filesystem::path& p);
};
