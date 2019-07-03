#include <sp2/io/virtualTouchKeys.h>
#include <sp2/io/keybinding.h>


namespace sp {
namespace io {

static int virtual_button_counter = 0;

VirtualTouchKeyLayer::VirtualTouchKeyLayer(int priority)
: sp::GraphicsLayer(priority)
{
}

void VirtualTouchKeyLayer::render(RenderQueue& queue) {}

bool VirtualTouchKeyLayer::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    for(unsigned int n=0; n<buttons.size(); n++)
    {
        if (buttons[n].area.contains(convertPosition(position)))
        {
            active_pointers[id] = n;
            Keybinding::setVirtualKey(buttons[n].virtual_key_index, 1);
            return true;
        }
    }
    return false;
}

void VirtualTouchKeyLayer::onPointerDrag(Vector2d position, int id)
{
    auto it = active_pointers.find(id);
    if (it != active_pointers.end())
    {
        if (buttons[it->second].area.contains(convertPosition(position)))
            return;
        Keybinding::setVirtualKey(buttons[it->second].virtual_key_index, 0);
    }
    for(unsigned int n=0; n<buttons.size(); n++)
    {
        if (buttons[n].area.contains(convertPosition(position)))
        {
            active_pointers[id] = n;
            Keybinding::setVirtualKey(buttons[n].virtual_key_index, 1);
            return;
        }
    }
}

void VirtualTouchKeyLayer::onPointerUp(Vector2d position, int id)
{
    auto it = active_pointers.find(id);
    if (it != active_pointers.end())
        Keybinding::setVirtualKey(buttons[it->second].virtual_key_index, 0);
}

void VirtualTouchKeyLayer::onTextInput(const string& text) {}
void VirtualTouchKeyLayer::onTextInput(TextInputEvent e) {}


void VirtualTouchKeyLayer::addButton(const Rect2f& area, Keybinding& key)
{
    addButton(area, getIndexForKey(key));
}

void VirtualTouchKeyLayer::addButton(const Rect2f& area, int virtual_key_index)
{
    buttons.emplace_back();
    VirtualButton& vb = buttons.back();
    vb.area = area;
    vb.virtual_key_index = virtual_key_index;
}

int VirtualTouchKeyLayer::getIndexForKey(Keybinding& key)
{
    key.addKey("virtual:" + string(virtual_button_counter));
    return virtual_button_counter++;
}

Vector2f VirtualTouchKeyLayer::convertPosition(const Vector2d& position) const
{
    return Vector2f(position.x / 2.0 + 0.5, position.y / -2.0 + 0.5);
}

};//namespace io
};//namespace sp
