#include <sp2/graphics/scene/basicnoderenderpass.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/camera.h>
#include <sp2/logging.h>

namespace sp {

BasicNodeRenderPass::BasicNodeRenderPass()
{
}

BasicNodeRenderPass::BasicNodeRenderPass(P<Camera> camera)
{
    addCamera(camera);
}

void BasicNodeRenderPass::addCamera(P<Camera> camera)
{
    cameras.add(camera);
}

void BasicNodeRenderPass::render(P<GraphicsLayer> layer, float aspect_ratio)
{
    if (!cameras.empty())
    {
        for(Camera* camera : cameras)
        {
            renderScene(camera->getScene(), camera, layer, aspect_ratio);
        }
    }else{
        for(Scene* scene : Scene::all())
        {
            renderScene(scene, nullptr, layer, aspect_ratio);
        }
    }
}

bool BasicNodeRenderPass::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    if (!cameras.empty())
    {
        for(Camera* camera : cameras)
        {
            if (privateOnPointerDown(camera->getScene(), camera, button, position, id))
                return true;
        }
    }
    else
    {
        for(Scene* scene : Scene::all())
        {
            if (privateOnPointerDown(scene, nullptr, button, position, id))
                return true;
        }
    }
    return false;
}

bool BasicNodeRenderPass::privateOnPointerDown(P<Scene> scene, P<Camera> camera, io::Pointer::Button button, Vector2d position, int id)
{
    if (!camera)
        camera = scene->getCamera();
    if (!camera)
        return false;
    if (scene->onPointerDown(button, pointerPositionToRay(camera, position), id))
    {
        pointer_scene[id] = scene;
        pointer_camera[id] = camera;
        return true;
    }
    return false;
}

void BasicNodeRenderPass::onPointerDrag(Vector2d position, int id)
{
    auto it = pointer_scene.find(id);
    if (it != pointer_scene.end() && it->second)
    {
        it->second->onPointerDrag(pointerPositionToRay(pointer_camera[id], position), id);
    }
}

void BasicNodeRenderPass::onPointerUp(Vector2d position, int id)
{
    auto it = pointer_scene.find(id);
    if (it != pointer_scene.end() && it->second)
    {
        it->second->onPointerUp(pointerPositionToRay(pointer_camera[id], position), id);
        pointer_scene.erase(it);
    }
}

void BasicNodeRenderPass::renderScene(P<Scene> scene, P<Camera> camera, P<GraphicsLayer> layer, float aspect_ratio)
{
    if (!camera)
        camera = scene->getCamera();

    if (scene->isEnabled() && camera)
    {
        camera->setAspectRatio(aspect_ratio);
        queue.clear();
        recursiveNodeRender(*scene->getRoot());
        queue.render(camera->getProjectionMatrix(), camera->getGlobalTransform().inverse());
    }
}

void BasicNodeRenderPass::recursiveNodeRender(Node* node)
{
    addNodeToRenderQueue(node);
    for(Node* child : node->getChildren())
    {
        recursiveNodeRender(child);
    }
}

Ray3d BasicNodeRenderPass::pointerPositionToRay(sp::P<sp::Camera> camera, Vector2d position)
{
    return Ray3d(camera->getGlobalTransform() * Vector3d(0, 0, 0), camera->getGlobalTransform() * camera->getProjectionMatrix().inverse() * Vector3d(position.x, position.y, -1));
}

void BasicNodeRenderPass::addNodeToRenderQueue(Node* node)
{
    if (node->render_data.type != sp::RenderData::Type::None && node->render_data.mesh)
        queue.add(node->getGlobalTransform(), node->render_data);
}

};//namespace sp
