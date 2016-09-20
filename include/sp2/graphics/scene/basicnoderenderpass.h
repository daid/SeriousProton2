#ifndef SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H
#define SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H

#include <sp2/graphics/scene/renderpass.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <map>

namespace sp {
class SceneNode;
class BasicNodeRenderPass : public RenderPass
{
public:
    BasicNodeRenderPass(string target_layer);
    
    virtual void render(sf::RenderTarget& target, P<GraphicsLayer> layer) override;
private:
    RenderQueue queue;
    void recursiveNodeRender(SceneNode* node);
};

};//!namespace sp

#endif//SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H
