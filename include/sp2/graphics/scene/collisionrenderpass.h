#ifndef SP2_GRAPHICS_SCENE_COLLISIONRENDERPASS_H
#define SP2_GRAPHICS_SCENE_COLLISIONRENDERPASS_H

#include <sp2/graphics/scene/renderpass.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <map>

namespace sp {
class SceneNode;
class CollisionRenderPass : public RenderPass
{
public:
    CollisionRenderPass(string target_layer);
    
    virtual void render(sf::RenderTarget& target, P<GraphicsLayer> layer) override;
private:
    RenderQueue queue;
};

};//!namespace sp

#endif//SP2_GRAPHICS_SCENE_COLLISIONRENDERPASS_H
