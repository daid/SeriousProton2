#ifndef SP2_IO_KEYBINDING_H
#define SP2_IO_KEYBINDING_H

#include <sp2/string.h>
#include <sp2/pointerList.h>
#include <sp2/io/pointer.h>
#include <SFML/Window/Event.hpp>

namespace sp {
class Engine;

namespace io {

class Keybinding : public AutoPointerObject
{
public:
    Keybinding(string name, string default_key = "");

    const string& getName() const { return name; }
    const string& getLabel() const { return label; }
    
    void setKey(string key);
    string getKey();
    
    bool get() const; //True when this key is currently being pressed.
    bool getDown() const; //True for 1 update cycle when the key is pressed.
    bool getUp() const; //True for 1 update cycle when the key is released.
    float getValue() const;
    
    static void loadKeybindings(const string& filename);
    static void saveKeybindings(const string& filename);
private:
    string name;
    string label;
    
    int key_number;

    float value;
    bool down_event;
    bool up_event;
    bool fixed_down_event;
    bool fixed_up_event;

    void setValue(float value);
    void postUpdate();
    void postFixedUpdate();
    
    static void allPostUpdate();
    static void allPostFixedUpdate();
    
    static void handleEvent(const sf::Event& event);
    static void updateKeys(int key_number, float value);
    
    static PList<Keybinding> keybindings;
    
    static constexpr int type_mask = (0x0f << 16);
    static constexpr int keyboard_mask = (1 << 16);
    static constexpr int pointer_mask =  (2 << 16);
    static constexpr int joystick_button_mask = (3 << 16);
    static constexpr int joystick_axis_mask = (4 << 16);
    
    friend class sp::Engine;
};

};//namespace io
};//namespace sp

#endif//SP2_IO_KEYBINDING_H
