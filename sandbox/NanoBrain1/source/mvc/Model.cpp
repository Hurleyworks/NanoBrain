#include "Model.h"
#include "mace_core/mace_core.h"

bool hasObjExtension (const std::filesystem::path& filePath)
{
    return filePath.extension() == ".obj";
}

bool isStaticBody (const std::filesystem::path& filePath)
{
    std::string filename = filePath.stem().string();

    // Check if filename starts with "static"
    return filename.rfind ("static", 0) == 0;
}

void Model::init (CameraHandle& camera, const std::string& resourceFolder, const std::string& repoFolder)
{
    // compile the optix kernels using NVCC
    nvcc.compile (resourceFolder, repoFolder);

    renderer.init (resourceFolder);
    renderer.setCamera (camera);

    // add environment hdr
    std::string hdrPath = resourceFolder + "/skydome.hdr";
    OIIO::ImageBuf hdr (hdrPath);
    renderer.addSkyDomeImage (std::move (hdr));

    // add ground plane
    std::filesystem::path ground (resourceFolder + "/static_textured_ground.obj");
    processPath (ground);

    // add a ball
    std::filesystem::path ball (resourceFolder + "/ball.obj");
    processPath (ball);
}

void Model::render()
{
    renderer.render();
}

void Model::updatePhysics()
{
    if (newton.update (engineState))
    {
        renderer.updateMotion();
    }

    // after a Reset, change the engine state back to Paused
    // and update the renderer
    if (engineState == PhysicsEngineState (PhysicsEngineState::Reset))
    {
        renderer.updateMotion();
        engineState = PhysicsEngineState (PhysicsEngineState::Paused);

        // the View's version of PhysicsEngineState is still set to Reset
        // so we need to set it to Paused
        physicsStateEmitter.fire (engineState);
    }
}

void Model::processPath (const std::filesystem::path& p)
{
    if (std::filesystem::is_directory (p))
    {
        for (const auto& entry : std::filesystem::directory_iterator (p))
        {
            processPath (entry.path());
        }
    }
    else
    {
        if (hasObjExtension (p))
        {
            OptiXGeometryRef g = OptiXTriangleMesh<Shared::Vertex, Shared::Triangle, Shared::GeometryData>::create();
            g->fromFile (p);

            OptiXNode node = OptiXRenderable::create();
            node->g = g;

            node->st.worldTransform.setIdentity();
            node->st.makeCurrentPoseStartPose();

            node->name = p.stem().string();

            if (isStaticBody (p))
            {
                node->desc.bodyType = BodyType::Static;
                node->desc.shape = CollisionShape::Mesh;
                node->desc.mass = 0.0f;
                node->st.worldTransform.translation() = Eigen::Vector3f (0.0, -1.0f, 0.0f);
                node->st.makeCurrentPoseStartPose();
            }
            else
            {
            }

            // add the node to the renderer
            renderer.addRenderableNode (node, p);

            // add a weak node to the physics engine
            newton.addBody (node, engineState);

            // add a stack of geomety instances to renderer
            // don't make static instances
            if (!node->isStaticBody())
            {
                uint32_t instanceCount = 50;
                GeometryInstances instances (instanceCount);
                renderer.addRenderableGeometryInstances (node, instances);

                // add to newton
                newton.addGeometryInstances (node, instances, engineState);
            }
        }
    }
}

void Model::onDrop (const std::vector<std::string>& filenames)
{
    for (const auto& filename : filenames)
    {
        std::filesystem::path p (filename);
        processPath (p);
    }
}
