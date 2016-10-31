#include <sp2/graphics/scene/basicnoderenderpass.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/cameraNode.h>
#include <sp2/logging.h>

namespace sp {

BasicNodeRenderPass::BasicNodeRenderPass(string target_layer)
: RenderPass(target_layer)
{
}
    
void BasicNodeRenderPass::render(sf::RenderTarget& target, P<GraphicsLayer> layer)
{
    for(Scene* scene : Scene::scenes)
    {
        P<CameraNode> camera = scene->getCamera();
        if (scene->isEnabled() && camera)
        {
            //TODO: Account for viewport, currently assumes viewport is on the whole target.
            camera->setAspectRatio(double(target.getSize().x) / double(target.getSize().y));
            queue.clear();
            recursiveNodeRender(*scene->getRoot());
            queue.render(camera->getProjectionMatrix(), camera->getGlobalTransform().inverse(), target);
        }
    }
}

void BasicNodeRenderPass::recursiveNodeRender(SceneNode* node)
{
    if (node->render_data.type != sp::RenderData::Type::None)
        queue.add(node->getGlobalTransform(), node->render_data);

    for(SceneNode* child : node->getChildren())
    {
        recursiveNodeRender(child);
    }
}

};//!namespace sp
