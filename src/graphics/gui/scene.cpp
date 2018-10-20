#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/widget/root.h>
#include <sp2/scene/camera.h>
#include <sp2/math/plane.h>


namespace sp {
namespace gui {

P<Scene> Scene::default_gui_scene;

Scene::Scene(Vector2d size, Direction fixed_direction, string scene_name, int priority)
: sp::Scene(scene_name, priority)
{
    if (!default_gui_scene)
        default_gui_scene = this;

    root_widget = new gui::RootWidget(getRoot(), size);
    setDefaultCamera(new Camera(getRoot()));
    switch(fixed_direction)
    {
    case Direction::Horizontal:
        getCamera()->setOrtographic(size.x / 2, Camera::Direction::Horizontal);
        break;
    case Direction::Vertical:
        getCamera()->setOrtographic(size.y / 2, Camera::Direction::Vertical);
        break;
    }
    getCamera()->setPosition(size * 0.5);
}

bool Scene::onPointerDown(io::Pointer::Button button, Ray3d ray, int id)
{
    Vector2d position(ray.end.x, ray.end.y);
    P<Widget> w = root_widget->getWidgetAt<Widget>(position);
    while(w)
    {
        if (w->onPointerDown(button, position - w->getGlobalPosition2D(), id))
        {
            if (focus_widget)
            {
                focus_widget->focus = false;
                focus_widget->markRenderDataOutdated();
            }
            if (w->focusable)
            {
                focus_widget = w;
                focus_widget->focus = true;
                focus_widget->markRenderDataOutdated();
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
        w = w->getParent();
    }
    if (focus_widget)
    {
        focus_widget->focus = false;
        focus_widget->markRenderDataOutdated();
        focus_widget = nullptr;
    }
    return false;
}

void Scene::onPointerDrag(Ray3d ray, int id)
{
    Vector2d position(ray.end.x, ray.end.y);
    auto it = pointer_widget.find(id);
    if (it != pointer_widget.end() && it->second)
        it->second->onPointerDrag(position - it->second->getGlobalPosition2D(), id);
}

void Scene::onPointerUp(Ray3d ray, int id)
{
    Vector2d position(ray.end.x, ray.end.y);
    auto it = pointer_widget.find(id);
    if (it != pointer_widget.end() && it->second)
    {
        it->second->onPointerUp(position - it->second->getGlobalPosition2D(), id);
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

P<RootWidget> Scene::getRootWidget()
{
    return root_widget;
}

};//namespace gui
};//namespace sp
