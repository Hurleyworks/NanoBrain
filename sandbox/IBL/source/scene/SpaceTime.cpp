
#include "SpaceTime.h"

// ctor
SpaceTime::SpaceTime()
{
	reset();
}

// reset
void SpaceTime::reset()
{
	birth = std::chrono::system_clock::now();

	startTransform.setIdentity();
	localTransform.setIdentity();
	worldTransform.setIdentity();

	modelBound.setEmpty();
	worldBound.setEmpty();

	scale = Vector3f::Ones();
    startScale = scale;
	
	modeledOffset = Vector3f::Zero();
	centerOfVertexMass = modeledOffset;
}

// getLifeSpan
std::chrono::duration<double> SpaceTime::getLifeSpan()
{
	auto now = std::chrono::system_clock::now();
	return now - birth;
}

// updateWorldBounds
void SpaceTime::updateWorldBounds(bool includeScale)
{
	AlignedBox3f b = modelBound;
	if(includeScale)
	{ 
		for (int i = 0; i < 3; i++)
		{
			b.max()[i] *= scale[i];
			b.min()[i] *= scale[i];
		}
	}
	b.translate(worldTransform.translation());

	worldBound = b;
}

// updateLocalBounds
void SpaceTime::updateLocalBounds(bool includeScale)
{
	if (includeScale)
	{
		Vector3f min, max;
		for (int i = 0; i < 3; i++)
		{
			min[i] = modelBound.min()[i] * scale[i];
			max[i] = modelBound.max()[i] * scale[i];
		}

		modelBound.min() = localTransform.translation() + min;
		modelBound.max() = localTransform.translation() + max;

		return;
	}

	// we don't want to apply the rotation here since this is an AABB, right?
	modelBound.min() = localTransform.translation() + modelBound.min();
	modelBound.max() = localTransform.translation() + modelBound.max();
}

// is2D
bool SpaceTime::is2D() const
{
    Vector3f dimensions = modelBound.sizes();
    int dim = 0;
    for (int i = 0; i < 3; i++)
    {
        if (dimensions[i] >= std::numeric_limits<float>::epsilon())
        {
            ++dim;
        }
    }
    return dim == 2;
}


