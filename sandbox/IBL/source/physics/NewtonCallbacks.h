
#pragma once

#include <ndNewton.h>
#include <ndContactCallback.h>

#include "../scene/RenderableNode.h"

class NewtonCallbacks : public ndBodyNotify
{
 public:
    NewtonCallbacks (OptiXWeakNode weakNode);
    ~NewtonCallbacks();

    void OnTransform (ndInt32 threadIndex, const ndMatrix& matrix) override;
    void OnApplyExternalForce (ndInt32 threadIndex, ndFloat32 timestep) override;
    void* GetUserData() const override
    {
        return weakNode.expired() ? nullptr : weakNode.lock().get();
    }

 private:
    OptiXWeakNode weakNode;
}; // end class NewtonCallbacks
