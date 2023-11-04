/*
MIT License

Copyright (c) 2023 Steve Hurley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <sabi_core/sabi_core.h>

using Eigen::Affine3f;
using Eigen::AlignedBox3f;
using Eigen::Vector3f;

class SpaceTime
{
 public:
    SpaceTime();
    ~SpaceTime() = default;

    Affine3f startTransform;
    Affine3f localTransform;
    Affine3f worldTransform;
 
    AlignedBox3f modelBound;
    AlignedBox3f worldBound;

    Vector3f scale;
    Vector3f startScale;

    Vector3f modeledOffset;
    Vector3f centerOfVertexMass;

    std::chrono::time_point<std::chrono::system_clock> birth;
    std::chrono::duration<double> getLifeSpan();

    void reset();

    void updateWorldBounds (bool includeScale = false);
    void updateLocalBounds (bool includeScale = false);
    void makeCurrentPoseStartPose() { startTransform = worldTransform; }
    void resetToStartPose() { worldTransform = startTransform; }
    bool is2D() const;

    void debug()
    {
        mace::matStr4f (startTransform, DBUG, "Start transform");
        mace::matStr4f (localTransform, DBUG, "Local transform");
        mace::matStr4f (worldTransform, DBUG, "World transform");

        mace::vecStr3f (scale, DBUG, "Scale");
        mace::vecStr3f (startScale, DBUG, "Start scale");

        mace::vecStr3f (modelBound.min(), DBUG, "ModelBound min");
        mace::vecStr3f (modelBound.max(), DBUG, "ModelBound max");

        mace::vecStr3f (worldBound.min(), DBUG, "WorldBound min");
        mace::vecStr3f (worldBound.max(), DBUG, "WorlddBound max");
    }

}; // end class SpaceTime
