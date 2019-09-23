#ifndef SP2_IO_VIRTUAL_TOUCH_KEYS_H
#define SP2_IO_VIRTUAL_TOUCH_KEYS_H

#include <sp2/graphics/graphicslayer.h>


namespace sp {
namespace io {

class Keybinding;

/**
    Class to handle areas of the screen as virtual keybinding keys.
    This is most useful on android where the input options are limited, but parts of the screen
    can be assigned to keysbindings.
    
    There are 3 types of bindings:
    * Simple keys, on/off pressed or not.
    * TODO: Single direction "joystick" like axis
            - With a pre-defined center point
            - With the "first hit" as center point
    * TODO: 2 direction "joystick" bound to 2 different keybindings.
 */
class VirtualTouchKeyLayer : public sp::GraphicsLayer
{
public:
    VirtualTouchKeyLayer(int priority);
    
    void addButton(const Rect2f& area, Keybinding& key);
    void addButton(const Rect2f& area, int virtual_key_index);

    virtual void render(RenderQueue& queue) override;
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
    virtual void onPointerDrag(Vector2d position, int id) override;
    virtual void onPointerUp(Vector2d position, int id) override;
    virtual void onTextInput(const string& text) override;
    virtual void onTextInput(TextInputEvent e) override;
private:
    int getIndexForKey(Keybinding& key);
    Vector2f convertPosition(const Vector2d& position) const;

    class IVirtualTouchKey
    {
    public:
        virtual bool onDown(Vector2f position) = 0;
        virtual bool onDrag(Vector2f position) = 0;
        virtual void onUp(Vector2f position) = 0;
    };
    class VirtualButton : public IVirtualTouchKey
    {
    public:
        VirtualButton(Rect2f area, int key_index);

        virtual bool onDown(Vector2f position) override;
        virtual bool onDrag(Vector2f position) override;
        virtual void onUp(Vector2f position) override;
    private:
        Rect2f area;
        int key_index;
    };

    std::vector<IVirtualTouchKey*> keys;
    std::unordered_map<int, int> active_pointers;
};

};//namespace io
};//namespace sp


#endif//SP2_IO_VIRTUAL_TOUCH_KEYS_H
