
#include "NewtonEngine.h"
#include "handlers/NewtonHandlers.h"
#include "NewtonWorld.h"

static ndUnsigned64 m_prevTime = 0;

// ctor
NewtonEngine::NewtonEngine()
{
    ctx = std::make_shared<PhysicsContext>();
    ctx->init();
}

// dtor
NewtonEngine::~NewtonEngine()
{
}

bool NewtonEngine::update (PhysicsEngineState state)
{
    ndFloat32 timestep = dGetElapsedSeconds();

    if (state == PhysicsEngineState (PhysicsEngineState::Running))
    {
        double updateTime = ctx->newtonWorld->advanceTime (timestep);
        return true;
    }
    else if (state == PhysicsEngineState (PhysicsEngineState::Reset))
    {
        resetEngine();
    }

    return false;
}

void NewtonEngine::addBody (OptiXWeakNode weakNode, PhysicsEngineState engineState)
{
    try
    {
        ctx->handlers->body->addBody (weakNode, engineState);
        ctx->weakNodes.push_back (weakNode);
    }
    catch (std::exception& e)
    {
        LOG (CRITICAL) << e.what();
    }
}

void NewtonEngine::addGeometryInstances (OptiXWeakNode instancedFrom, GeometryInstances& instances, PhysicsEngineState engineState)
{
    try
    {
        ctx->handlers->body->addGeometryInstances (instancedFrom, instances, engineState);

        for (auto& node : instances)
        {
            ctx->weakNodes.push_back (node);
        }
    }
    catch (std::exception& e)
    {
        LOG (CRITICAL) << e.what();
    }
}

void NewtonEngine::dResetTimer()
{
    m_prevTime = ndGetTimeInMicroseconds();
}

void NewtonEngine::resetEngine()
{
    ctx->newtonWorld->Sync();
    ctx->newtonWorld->ClearCache();

    for (auto& n : ctx->weakNodes)
    {
        if (n.expired()) continue;

        OptiXNode node = n.lock();

        node->st.resetToStartPose();

        ndBodyDynamic* const ndBody = static_cast<ndBodyDynamic*> (node->getUserdata());
        if (!ndBody) continue;

        // 4th component not zero was cause of not working
        ndVector zero = ndVector (0.0f, 0.0f, 0.0f, 0.0f);

        ndBody->SetForce (zero);
        ndBody->SetTorque (zero);
        ndBody->SetAccel (zero);
        ndBody->SetAlpha (zero);
        ndBody->SetVelocity (zero);
        ndBody->SetOmega (zero);
        ndBody->SetSleepState (1);

        ndMatrix startPose;
        eigenToNewton (node->st.startTransform, startPose);
        ndBody->SetMatrix (startPose);

        ndShapeInstance& shape = ndBody->GetCollisionShape();
        Eigen::Vector3f s = node->st.startScale;

        shape.SetScale (ndVector (s.x(), s.y(), s.z(), 1.0f));

        ndBody->SetMassMatrix (node->desc.mass, shape);
    }
}

ndFloat32 NewtonEngine::dGetElapsedSeconds()
{
    const ndFloat64 TICKS2SEC = 1.0e-6f;
    ndUnsigned64 microseconds = ndGetTimeInMicroseconds();

    ndFloat32 timeStep = ndFloat32 ((ndFloat64)(microseconds - m_prevTime) * TICKS2SEC);
    m_prevTime = microseconds;

    return timeStep;
}