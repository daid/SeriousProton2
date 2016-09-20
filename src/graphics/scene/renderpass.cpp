#include <sp2/graphics/scene/renderpass.h>

namespace sp {

RenderPass::RenderPass(string target_layer)
: target_layer(target_layer)
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
