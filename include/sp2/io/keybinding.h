#ifndef SP2_IO_KEYBINDING_H
#define SP2_IO_KEYBINDING_H

#include <sp2/string.h>
#include <sp2/pointerList.h>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>

namespace sp {
class Engine;

namespace io {

class Keybinding : public AutoPointerObject
{
public:
    Keybinding(string name);

    const string& getName() const { return name; }
    const string& getLabel() const { return label; }
    
    void setKey(sf::Keyboard::Key key);
    
    bool get() const;
    bool getDown() const; //True for 1 update cycle when the key is pressed.
    bool getUp() const; //True for 1 update cycle when the key is released.
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
    
    float value;
    float previous_value;

    void update();
    static void updateAll();
    static PList<Keybinding> keybindings;
    
    friend class sp::Engine;
};

};//!namespace io
};//!namespace sp

#endif//SP2_IO_KEYBINDING_H
