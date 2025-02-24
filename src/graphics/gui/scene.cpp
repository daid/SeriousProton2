#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/widget/root.h>
#include <sp2/scene/camera.h>
#include <sp2/math/plane.h>

#include <SDL3/SDL.h>


namespace sp {
namespace gui {

P<Scene> Scene::default_gui_scene;

Scene::Scene(Vector2d size, const string& scene_name, int priority)
: sp::Scene(scene_name, priority), size(size)
{
    if (!default_gui_scene)
        default_gui_scene = this;

    root_widget = new gui::RootWidget(getRoot(), size);
    camera = new Camera(getRoot());
    camera->setOrtographic(size * 0.5);
    camera->setPosition(size * 0.5);
    setDefaultCamera(camera);
}

void Scene::onUpdate(float delta)
{
    if (stretch && camera)
    {
        auto projection_matrix = camera->getProjectionMatrix();
        double render_aspect_ratio = projection_matrix.data[5] / projection_matrix.data[0];
        double requested_aspect_radio = size.x / size.y;
        
        root_widget->gui_size = size;
        if (render_aspect_ratio > requested_aspect_radio)
            root_widget->gui_size.x = size.y * render_aspect_ratio;
        else
            root_widget->gui_size.y = size.x / render_aspect_ratio;
        camera->setPosition(root_widget->gui_size * 0.5);
    }
}

bool Scene::onPointerMove(Ray3d ray, int id)
{
    Vector2d position(ray.end.x, ray.end.y);
    Vector2d local_position;
    P<Widget> w = root_widget->getWidgetAt<Widget>(position, &local_position);
    while(w)
    {
        if (w->onPointerMove(local_position, id))
        {
            auto it = pointer_widget.find(id);
            if (it != pointer_widget.end() && it->second && it->second != w)
            {
                pointer_widget[id]->hover = false;
                pointer_widget[id]->markRenderDataOutdated();
                pointer_widget[id]->onPointerLeave(id);

                pointer_widget[id] = w;
                w->hover = true;
                w->markRenderDataOutdated();
            }
            return true;
        }
        local_position = w->getLocalPoint2D(local_position);
        w = w->getParent();
    }
    return false;
}

void Scene::onPointerLeave(int id)
{
    auto it = pointer_widget.find(id);
    if (it != pointer_widget.end() && it->second)
    {
        pointer_widget[id]->hover = false;
        pointer_widget[id]->markRenderDataOutdated();
        pointer_widget[id]->onPointerLeave(id);
        pointer_widget.erase(it);
    }
}

bool Scene::onPointerDown(io::Pointer::Button button, Ray3d ray, int id)
{
    Vector2d position(ray.end.x, ray.end.y);
    Vector2d local_position;
    P<Widget> w = root_widget->getWidgetAt<Widget>(position, &local_position);
    while(w)
    {
        if (w->onPointerDown(button, local_position, id))
        {
            if (focus_widget)
            {
                focus_widget->focus = false;
                focus_widget->markRenderDataOutdated();
                if (focus_widget->enableTextInputOnFocus())
                    SDL_StopTextInput(nullptr); //TODO
            }
            if (w->focusable)
            {
                focus_widget = w;
                focus_widget->focus = true;
                focus_widget->markRenderDataOutdated();
                if (focus_widget->enableTextInputOnFocus())
                    SDL_StartTextInput(nullptr); //TODO
            }
            else
            {
                focus_widget = nullptr;
            }
            pointer_widget[id] = w;
            w->hover = true;
            w->markRenderDataOutdated();
            return true;
        }
        local_position = w->getLocalPoint2D(local_position);
        w = w->getParent();
    }
    if (focus_widget)
    {
        focus_widget->focus = false;
        focus_widget->markRenderDataOutdated();
        if (focus_widget->enableTextInputOnFocus())
            SDL_StopTextInput(nullptr); //TODO
        focus_widget = nullptr;
    }
    return false;
}

void Scene::onPointerDrag(Ray3d ray, int id)
{
    Vector2d position(ray.end.x, ray.end.y);
    auto it = pointer_widget.find(id);
    if (it != pointer_widget.end() && it->second)
        it->second->onPointerDrag(sp::Vector2d(it->second->getGlobalTransform().inverse() * sp::Vector2f(position)), id);
}

void Scene::onPointerUp(Ray3d ray, int id)
{
    Vector2d position(ray.end.x, ray.end.y);
    auto it = pointer_widget.find(id);
    if (it != pointer_widget.end() && it->second)
    {
        it->second->onPointerUp(sp::Vector2d(it->second->getGlobalTransform().inverse() * sp::Vector2f(position)), id);
        if (it->second) //The widget might have been deleted by the pointer-up event.
        {
            bool dehover = true;
            for(auto i : pointer_widget)
                if (i.first != id && i.second == it->second)
                    dehover = false;
            if (dehover)
            {
                it->second->hover = false;
                it->second->markRenderDataOutdated();
            }
        }
        pointer_widget.erase(it);
    }
}

bool Scene::onWheelMove(Ray3d ray, io::Pointer::Wheel direction)
{
    Vector2d position(ray.end.x, ray.end.y);
    Vector2d local_position;
    P<Widget> w = root_widget->getWidgetAt<Widget>(position, &local_position);
    while(w)
    {
        if (w->onWheelMove(local_position, direction))
            return true;
        local_position = w->getLocalPoint2D(local_position);
        w = w->getParent();
    }
    return false;
}

void Scene::onTextInput(const string& text)
{
    if (focus_widget)
        focus_widget->onTextInput(text);
}

void Scene::onTextInput(TextInputEvent e)
{
    if (focus_widget)
        focus_widget->onTextInput(e);
}

P<Widget> Scene::getRootWidget()
{
    return root_widget;
}

}//namespace gui
}//namespace sp
