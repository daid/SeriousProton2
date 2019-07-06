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
    for(unsigned int n=0; n<keys.size(); n++)
    {
        if (keys[n]->onDown(convertPosition(position)))
        {
            active_pointers[id] = n;
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
        if (keys[it->second]->onDrag(convertPosition(position)))
            return;
        active_pointers.erase(it);
    }
    for(unsigned int n=0; n<keys.size(); n++)
    {
        if (keys[n]->onDown(convertPosition(position)))
        {
            active_pointers[id] = n;
            return;
        }
    }
}

void VirtualTouchKeyLayer::onPointerUp(Vector2d position, int id)
{
    auto it = active_pointers.find(id);
    if (it != active_pointers.end())
    {
        keys[it->second]->onUp(convertPosition(position));
        active_pointers.erase(it);
    }
}

void VirtualTouchKeyLayer::onTextInput(const string& text) {}
void VirtualTouchKeyLayer::onTextInput(TextInputEvent e) {}


void VirtualTouchKeyLayer::addButton(const Rect2f& area, Keybinding& key)
{
    addButton(area, getIndexForKey(key));
}

void VirtualTouchKeyLayer::addButton(const Rect2f& area, int virtual_key_index)
{
    keys.push_back(new VirtualButton(area, virtual_key_index));
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

VirtualTouchKeyLayer::VirtualButton::VirtualButton(Rect2f area, int key_index)
: area(area), key_index(key_index)
{

}

bool VirtualTouchKeyLayer::VirtualButton::onDown(Vector2f position)
{
    if (!area.contains(position))
        return false;
    Keybinding::setVirtualKey(key_index, 1);
    return true;
}

bool VirtualTouchKeyLayer::VirtualButton::onDrag(Vector2f position)
{
    if (!area.contains(position))
    {
        Keybinding::setVirtualKey(key_index, 0);
        return false;
    }
    return true;
}

void VirtualTouchKeyLayer::VirtualButton::onUp(Vector2f position)
{
    Keybinding::setVirtualKey(key_index, 0);
}


};//namespace io
};//namespace sp
