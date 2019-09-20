#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/widget/root.h>
#include <sp2/scene/camera.h>
#include <sp2/math/plane.h>

#include <SDL.h>


namespace sp {
namespace gui {

P<Scene> Scene::default_gui_scene;

Scene::Scene(Vector2d size, string scene_name, int priority)
: sp::Scene(scene_name, priority), size(size)
{
    if (!default_gui_scene)
        default_gui_scene = this;

    root_widget = new gui::RootWidget(getRoot(), size);
    setDefaultCamera(new Camera(getRoot()));
    getCamera()->setOrtographic(size * 0.5);
    getCamera()->setPosition(size * 0.5);
}

void Scene::onUpdate(float delta)
{
    if (stretch && getCamera())
    {
        auto projection_matrix = getCamera()->getProjectionMatrix();
        double render_aspect_ratio = projection_matrix.data[5] / projection_matrix.data[0];
        double requested_aspect_radio = size.x / size.y;
        
        root_widget->gui_size = size;
        if (render_aspect_ratio > requested_aspect_radio)
            root_widget->gui_size.x = size.y * render_aspect_ratio;
        else
            root_widget->gui_size.y = size.x / render_aspect_ratio;
        getCamera()->setPosition(root_widget->gui_size * 0.5);
    }
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
                if (focus_widget->enableTextInputOnFocus())
                    SDL_StopTextInput();
            }
            if (w->focusable)
            {
                focus_widget = w;
                focus_widget->focus = true;
                focus_widget->markRenderDataOutdated();
                if (focus_widget->enableTextInputOnFocus())
                    SDL_StartTextInput();
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
        if (focus_widget->enableTextInputOnFocus())
            SDL_StopTextInput();
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

P<RootWidget> Scene::getRootWidget()
{
    return root_widget;
}

};//namespace gui
};//namespace sp
