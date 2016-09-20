#include <sp2/graphics/scene/basicnoderenderpass.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>

namespace sp {

BasicNodeRenderPass::BasicNodeRenderPass(string target_layer)
: RenderPass(target_layer)
{
}
    
void BasicNodeRenderPass::render(sf::RenderTarget& target, P<GraphicsLayer> layer)
{
    queue.clear();
    for(Scene* scene : Scene::scenes)
    {
        if (scene->isEnabled())
        {
            recursiveNodeRender(*scene->getRoot());
        }
    }
}

void BasicNodeRenderPass::recursiveNodeRender(SceneNode* node)
{
    if (node->render_data)
        queue.add(node->getGlobalTransform(), *node->render_data);

    for(SceneNode* child : node->getChildren())
    {
        recursiveNodeRender(child);
    }
}

};//!namespace sp
