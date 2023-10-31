
#include "NewtonBodyHandler.h"
#include "NewtonHandlers.h"
#include "../NewtonCallbacks.h"
#include "../NewtonWorld.h"

// ctor
NewtonBodyHandler::NewtonBodyHandler (PhysicsContextPtr ctx) :
    ctx (ctx)
{
}

// dtor
NewtonBodyHandler::~NewtonBodyHandler()
{
}

void NewtonBodyHandler::addBody (OptiXWeakNode weakNode, PhysicsEngineState engineState)
{
    if (weakNode.expired()) return;

    // if the engine is running, bodies have to be added on Newton's thread
    if (engineState == PhysicsEngineState::Running)
        pendingAdds.enqueue (weakNode.lock());
    else
        addBodyToEngine (weakNode);
}

void NewtonBodyHandler::addGeometryInstances (OptiXWeakNode instancedFrom, GeometryInstances& instances, PhysicsEngineState engineState)
{
    if (instancedFrom.expired()) return;

    OptiXNode fromNode = instancedFrom.lock();
    ndBodyDynamic* const fromBody = static_cast<ndBodyDynamic*> (fromNode->getUserdata());

    if (engineState != PhysicsEngineState::Running)
    {
        for (auto& node : instances)
        {
            ndMatrix startPose;
            eigenToNewton (node->st.worldTransform, startPose);

            ndShapeInstance shapeInst = fromBody->GetAsBodyKinematic()->GetCollisionShape();

            // set the scale
            Scale s = node->st.scale;
            ndVector scale = ndVector (s.x(), s.y(), s.z(), 0.0f);
            shapeInst.SetScale (scale);

            ndBodyDynamic* const body = new ndBodyDynamic();
            body->SetCollisionShape (shapeInst);
            body->SetMassMatrix (node->desc.mass, shapeInst);
            node->setUserData (body);
            body->SetMatrix (startPose);

            body->SetNotifyCallback (new NewtonCallbacks (node));

            ndSharedPtr<ndBody> bodyPtr (body);
            ctx->newtonWorld->AddBody (bodyPtr);
        }
    }
    else
    {
        for (auto& node : instances)
        {
            pendingGeometryInstances.enqueue (node);
        }
    }
}

void NewtonBodyHandler::onPostUpdate (ndFloat32 timestep)
{
    while (pendingAdds.size_approx())
    {
        OptiXNode node;
        bool found = pendingAdds.try_dequeue (node);
        if (found)
            addBodyToEngine (node);
    }

    while (pendingGeometryInstances.size_approx())
    {
        OptiXNode node;
        bool found = pendingGeometryInstances.try_dequeue (node);
        if (found)
            addGeometryInstanceToEngine (node->instancedFrom, node);
    }
}

void NewtonBodyHandler::addBodyToEngine (OptiXWeakNode weakNode)
{
    if (weakNode.expired()) return;

    OptiXNode node = weakNode.lock();
    ndMatrix startPose;
    eigenToNewton (node->st.worldTransform, startPose);

    ndShapeInstance shapeInst = ctx->handlers->ops->createCollisionShape (weakNode);
    if (shapeInst.GetShape() == nullptr)
        throw std::runtime_error ("Failed to create a collision shape for " + node->name);

    // set the scale
    Scale s = node->st.scale;
    ndVector scale = ndVector (s.x(), s.y(), s.z(), 0.0f);
    shapeInst.SetScale (scale);

    ndBodyDynamic* const body = new ndBodyDynamic();
    body->SetCollisionShape (shapeInst);
    body->SetMassMatrix (node->desc.mass, shapeInst);
    node->setUserData (body);
    body->SetMatrix (startPose);

    body->SetNotifyCallback (new NewtonCallbacks (node));

    ndSharedPtr<ndBody> bodyPtr (body);
    ctx->newtonWorld->AddBody (bodyPtr);
}

void NewtonBodyHandler::addGeometryInstanceToEngine (OptiXWeakNode instanceFrom, OptiXWeakNode weakNode)
{
    if (weakNode.expired() || instanceFrom.expired()) return;

    OptiXNode node = weakNode.lock();
    OptiXNode fromNode = instanceFrom.lock();

    ndMatrix startPose;
    eigenToNewton (node->st.worldTransform, startPose);

    ndBodyDynamic* const fromBody = static_cast<ndBodyDynamic*> (fromNode->getUserdata());

    ndShapeInstance shapeInst = fromBody->GetAsBodyKinematic()->GetCollisionShape();

    // set the scale
    Scale s = node->st.scale;
    ndVector scale = ndVector (s.x(), s.y(), s.z(), 0.0f);
    shapeInst.SetScale (scale);

    ndBodyDynamic* const body = new ndBodyDynamic();
    body->SetCollisionShape (shapeInst);
    body->SetMassMatrix (node->desc.mass, shapeInst);
    node->setUserData (body);
    body->SetMatrix (startPose);

    body->SetNotifyCallback (new NewtonCallbacks (node));

    ndSharedPtr<ndBody> bodyPtr (body);
    ctx->newtonWorld->AddBody (bodyPtr);
}
