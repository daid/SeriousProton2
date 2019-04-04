#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/camera.h>
#include <sp2/engine.h>
#include <sp2/logging.h>
#include <sp2/assert.h>


namespace sp {

std::unordered_map<string, P<Scene>> Scene::scene_mapping;
PList<Scene> Scene::scenes;

Scene::Scene(string scene_name, int priority)
: scene_name(scene_name), priority(priority)
{
    root = new Node(this);
    enabled = true;

    sp2assert(scene_mapping.find(scene_name) == scene_mapping.end(), "Cannot create two scenes with the same name.");
    scene_mapping[scene_name] = this;
    
    scenes.add(this);
    scenes.sort([](const P<Scene>& a, const P<Scene>& b){
        return a->priority - b->priority;
    });
}

Scene::~Scene()
{
    root.destroy();
    if (collision_backend)
        delete collision_backend;

    scene_mapping.erase(scene_name);
}

void Scene::setEnabled(bool enabled)
{
    if (this->enabled != enabled)
    {
        this->enabled = enabled;
        if (enabled)
            onEnable();
        else
            onDisable();
    }
}

void Scene::setDefaultCamera(P<Camera> camera)
{
    sp2assert(camera->getScene() == this, "Trying to set camera from different scene as default for scene.");
    this->camera = camera;
}

void Scene::update(float delta)
{
    if (root)
        updateNode(delta, *root);
    onUpdate(delta);
}

bool Scene::onPointerDown(io::Pointer::Button button, Ray3d ray, int id)
{
    return false;
}

void Scene::onPointerDrag(Ray3d ray, int id)
{
}

void Scene::onPointerUp(Ray3d ray, int id)
{
}

void Scene::fixedUpdate()
{
    if (collision_backend)
    {
        collision_backend->step(Engine::fixed_update_delta);
        collision_backend->postUpdate(0);
    }
    if (root)
        fixedUpdateNode(*root);
    onFixedUpdate();
}

void Scene::postFixedUpdate(float delta)
{
    if (collision_backend)
        collision_backend->postUpdate(delta);
}

void Scene::updateNode(float delta, P<Node> node)
{
    node->onUpdate(delta);
    if (node && node->animation)
        node->animation->update(delta, node->render_data);
    if (node)
    {
        for(Node* child : node->children)
            updateNode(delta, child);
    }
}

void Scene::fixedUpdateNode(P<Node> node)
{
    node->onFixedUpdate();
    if (node)
    {
        for(Node* child : node->children)
            fixedUpdateNode(child);
    }
}

void Scene::queryCollision(sp::Vector2d position, double range, std::function<bool(P<Node> object)> callback_function)
{
    if (!collision_backend)
        return;
    collision_backend->query(position, range, callback_function);
}

void Scene::queryCollision(sp::Vector2d position, std::function<bool(P<Node> object)> callback_function)
{
    if (!collision_backend)
        return;
    collision_backend->query(position, callback_function);
}

void Scene::queryCollision(Rect2d area, std::function<bool(P<Node> object)> callback_function)
{
    if (!collision_backend)
        return;
    collision_backend->query(area, callback_function);
}

void Scene::queryCollisionAny(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function)
{
    if (!collision_backend)
        return;
    collision_backend->queryAny(ray, callback_function);
}

void Scene::queryCollisionAll(Ray2d ray, std::function<bool(P<Node> object, Vector2d hit_location, Vector2d hit_normal)> callback_function)
{
    if (!collision_backend)
        return;
    collision_backend->queryAll(ray, callback_function);
}

};//namespace sp
