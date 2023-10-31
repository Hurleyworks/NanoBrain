#include "RenderableNode.h"

OptiXRenderable::OptiXRenderable()
{
}

OptiXRenderable::~OptiXRenderable()
{
    instance.destroy();
}
