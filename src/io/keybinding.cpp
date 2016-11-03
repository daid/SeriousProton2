#include <sp2/io/keybinding.h>

namespace sp {
namespace io {

PVector<Keybinding> Keybinding::keybindings;

Keybinding::Keybinding(string name)
: name(name), label(name)
{
    key = sf::Keyboard::Unknown;
    joystick_index = -1;
    joystick_button_index = -1;
    joystick_axis_enabled = false;
    joystick_axis = sf::Joystick::X;
    joystick_axis_positive = true;
    
    keybindings.add(this);
}
    
float Keybinding::get() const
{
    if (key != sf::Keyboard::Unknown)
    {
        if (sf::Keyboard::isKeyPressed(key))
            return 1.0;
    }
    if (joystick_index != -1)
    {
        if (joystick_button_index != -1 && sf::Joystick::isButtonPressed(joystick_index, joystick_button_index))
            return 1.0;
        if (joystick_axis_enabled)
        {
            float f = sf::Joystick::getAxisPosition(joystick_index, joystick_axis);
            if (!joystick_axis_positive)
                f = -f;
            if (f > 0.0)
                return f / 100.0f;
        }
    }
    return 0.0;
}

};//!namespace io
};//!namespace sp
