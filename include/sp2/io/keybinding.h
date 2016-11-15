#ifndef SP2_IO_KEYBINDING_H
#define SP2_IO_KEYBINDING_H

#include <sp2/string.h>
#include <sp2/pointerVector.h>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>

namespace sp {
namespace io {

class Keybinding : public AutoPointerObject
{
public:
    Keybinding(string name);

    const string& getName() const { return name; }
    const string& getLabel() const { return label; }
    
    void setKey(sf::Keyboard::Key key);
    
    bool get() const;
    float getValue() const;
    
    static void loadKeybindings(const string& filename);
    static void saveKeybindings(const string& filename);
private:
    string name;
    string label;
    
    sf::Keyboard::Key key;
    int joystick_index;
    int joystick_button_index;
    bool joystick_axis_enabled;
    sf::Joystick::Axis joystick_axis;
    bool joystick_axis_positive;

    static PVector<Keybinding> keybindings;
};

};//!namespace io
};//!namespace sp

#endif//SP2_IO_KEYBINDING_H
