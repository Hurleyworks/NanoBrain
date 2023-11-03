

#include "NewtonCallbacks.h"

using Eigen::Matrix3f;
using Eigen::Quaternionf;

// ctor
NewtonCallbacks::NewtonCallbacks (OptiXWeakNode weakNode) :
    ndBodyNotify (ndVector (ndFloat32 (0.0f), -10.0f, ndFloat32 (0.0f), ndFloat32 (0.0f))),
    weakNode (weakNode)
{
}

// dtor
NewtonCallbacks::~NewtonCallbacks()
{
}

void NewtonCallbacks::OnTransform (ndInt32 threadIndex, const ndMatrix& matrix)
{
    if (weakNode.expired()) return;

    OptiXNode node = weakNode.lock();

    ndBody* const body = GetBody();
    ndMatrix t (matrix);
    ndQuaternion r (body->GetRotation());

    Quaternionf q (r.m_w, r.m_x, r.m_y, r.m_z);
    node->st.worldTransform.translation() = Vector3f (t.m_posit.m_x, t.m_posit.m_y, t.m_posit.m_z);
    node->st.worldTransform.linear() = Matrix3f (q);

    ndBodyKinematic* const kinematic = body->GetAsBodyKinematic();
    if (kinematic)
    {
        // FIXME make this real
        float cutOff = -20.0f;
        if (node->st.worldTransform.translation().y() < cutOff)
        {
            // put them to sleep if they're out of sight
            kinematic->SetSleepState (1);
        }
        node->desc.sleepState = kinematic->GetSleepState();
    }
}

void NewtonCallbacks::OnApplyExternalForce (ndInt32 threadIndex, ndFloat32 timestep)
{
    if (weakNode.expired()) return;

    OptiXNode node = weakNode.lock();

    ndBodyKinematic* const body = GetBody()->GetAsBodyKinematic();

    if (body && body->GetInvMass() > 0.0f)
    {
        ndVector massMatrix (body->GetMassMatrix());
        ndVector force (GetGravity().Scale (massMatrix.m_w));
        body->SetForce (force);
        body->SetTorque (ndVector::m_zero);
    }

    ndBodyKinematic* const kinematic = body->GetAsBodyKinematic();
    if (kinematic)
    {
        node->desc.sleepState = kinematic->GetSleepState();
    }
}
