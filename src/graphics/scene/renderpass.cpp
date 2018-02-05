#include <sp2/graphics/scene/renderpass.h>

namespace sp {

RenderPass::RenderPass(string target_layer)
: target_layer(target_layer)
{
}

bool RenderPass::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    return false;
}

void RenderPass::onPointerDrag(Vector2d position, int id)
{
}

void RenderPass::onPointerUp(Vector2d position, int id)
{
}

string RenderPass::getTargetLayer() const
{
    return target_layer;
}

std::vector<string> RenderPass::getSourceLayers() const
{
    return std::vector<string>();
}

};//!namespace sp
