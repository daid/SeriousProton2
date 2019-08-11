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

void BasicNodeRenderPass::render(RenderQueue& queue)
{
    if (!cameras.empty())
    {
        for(P<Camera> camera : cameras)
            renderScene(queue, camera->getScene(), camera);
    }
    else
    {
        const auto& all_scenes = Scene::all();
        for(auto it = all_scenes.rbegin(); it != all_scenes.rend(); ++it)
            renderScene(queue, *it, nullptr);
    }
}

bool BasicNodeRenderPass::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    if (!cameras.empty())
    {
        for(P<Camera> camera : cameras)
        {
            if (privateOnPointerDown(camera->getScene(), camera, button, position, id))
                return true;
        }
    }
    else
    {
        for(P<Scene> scene : Scene::all())
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
        focus_scene = scene;
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

void BasicNodeRenderPass::onTextInput(const string& text)
{
    if (focus_scene)
        focus_scene->onTextInput(text);
}

void BasicNodeRenderPass::onTextInput(TextInputEvent e)
{
    if (focus_scene)
        focus_scene->onTextInput(e);
}

void BasicNodeRenderPass::renderScene(RenderQueue& queue, P<Scene> scene, P<Camera> camera)
{
    if (!camera)
        camera = scene->getCamera();

    if (scene->isEnabled() && camera)
    {
        queue.setCamera(camera);
        recursiveNodeRender(queue, scene->getRoot());
    }
}

void BasicNodeRenderPass::recursiveNodeRender(RenderQueue& queue, P<Node> node)
{
    addNodeToRenderQueue(queue, node);
    for(P<Node> child : node->getChildren())
    {
        recursiveNodeRender(queue, child);
    }
}

Ray3d BasicNodeRenderPass::pointerPositionToRay(sp::P<sp::Camera> camera, Vector2d position)
{
    const auto project_inv = camera->getProjectionMatrix().inverse();
    const auto transform = camera->getGlobalTransform();
    return Ray3d(
        Vector3d(transform * (project_inv.applyDirection(Vector3f(position.x, position.y, 0)))),
        Vector3d(transform * (project_inv * Vector3f(position.x, position.y, 0))));
}

void BasicNodeRenderPass::addNodeToRenderQueue(RenderQueue& queue, P<Node>& node)
{
    if (node->render_data.type != sp::RenderData::Type::None && node->render_data.mesh)
        queue.add(node->getGlobalTransform(), node->render_data);
}

};//namespace sp
