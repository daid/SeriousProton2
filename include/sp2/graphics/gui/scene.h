#ifndef SP2_GRAPHICS_GUI_SCENE_H
#define SP2_GRAPHICS_GUI_SCENE_H

#include <sp2/scene/scene.h>

namespace sp {
namespace gui {

class Widget;
class RootWidget;
class Scene : public sp::Scene
{
public:
    Scene(Vector2d size, const string& scene_name="GUI", int priority=100);

    virtual void onUpdate(float delta) override;
    virtual bool onPointerMove(Ray3d ray, int id) override;
    virtual void onPointerLeave(int id) override;
    virtual bool onPointerDown(io::Pointer::Button button, Ray3d ray, int id) override;
    virtual void onPointerDrag(Ray3d ray, int id) override;
    virtual void onPointerUp(Ray3d ray, int id) override;
    virtual void onTextInput(const string& text) override;
    virtual void onTextInput(TextInputEvent e) override;
    
    P<RootWidget> getRootWidget();
private:
    static P<Scene> default_gui_scene;
    
    P<RootWidget> root_widget;
    std::map<int, P<Widget>> pointer_widget;
    P<Widget> focus_widget;

    bool stretch = true;
    Vector2d size;

    friend class Loader;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_LOADER_H
