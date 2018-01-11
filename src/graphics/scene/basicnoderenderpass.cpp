#include <sp2/graphics/scene/basicnoderenderpass.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/camera.h>
#include <sp2/logging.h>

namespace sp {

BasicNodeRenderPass::BasicNodeRenderPass(string target_layer)
: RenderPass(target_layer)
{
}

BasicNodeRenderPass::BasicNodeRenderPass(string target_layer, P<Scene> scene)
: RenderPass(target_layer), single_scene(scene)
{
}

BasicNodeRenderPass::BasicNodeRenderPass(string target_layer, P<Scene> scene, P<Camera> camera)
: RenderPass(target_layer), single_scene(scene), specific_camera(camera)
{
}


void BasicNodeRenderPass::setScene(P<Scene> scene)
{
    single_scene = scene;
}

void BasicNodeRenderPass::setCamera(P<Camera> camera)
{
    specific_camera = camera;
}
    
void BasicNodeRenderPass::render(sf::RenderTarget& target, P<GraphicsLayer> layer, float aspect_ratio)
{
    if (single_scene)
    {
        renderScene(*single_scene, target, layer, aspect_ratio);
    }else{
        for(Scene* scene : Scene::scenes)
        {
            renderScene(scene, target, layer, aspect_ratio);
        }
    }
}

void BasicNodeRenderPass::renderScene(Scene* scene, sf::RenderTarget& target, P<GraphicsLayer> layer, float aspect_ratio)
{
    P<Camera> camera = scene->getCamera();
    if (specific_camera && specific_camera->getScene() == scene)
        camera = specific_camera;
    
    if (scene->isEnabled() && camera)
    {
        camera->setAspectRatio(aspect_ratio);
        queue.clear();
        recursiveNodeRender(*scene->getRoot());
        queue.render(camera->getProjectionMatrix(), camera->getGlobalTransform().inverse(), target);
    }
}

void BasicNodeRenderPass::recursiveNodeRender(Node* node)
{
    if (node->render_data.type != sp::RenderData::Type::None)
        queue.add(node->getGlobalTransform(), node->render_data);

    for(Node* child : node->getChildren())
    {
        recursiveNodeRender(child);
    }
}

};//!namespace sp
