#ifndef SP2_IO_KEYBINDING_H
#define SP2_IO_KEYBINDING_H

#include <sp2/string.h>
#include <sp2/pointerList.h>
#include <sp2/io/pointer.h>

//Forward declare the event structure, so we can pass it by reference to the keybindings, without exposing all SDL stuff.
union SDL_Event;

namespace sp {
class Engine;

namespace io {

class Keybinding : public AutoPointerObject
{
public:
    Keybinding(string name);
    Keybinding(string name, string default_key);
    Keybinding(string name, string default_key, string alternative_default_key);

    const string& getName() const { return name; }
    const string& getLabel() const { return label; }
    
    void setKey(string key);
    string getKey(int index);
    void addKey(string key);
    void clearKeys();
    
    bool isBound();
    
    bool get() const; //True when this key is currently being pressed.
    bool getDown() const; //True for 1 update cycle when the key is pressed.
    bool getUp() const; //True for 1 update cycle when the key is released.
    float getValue() const; //Returns a value in the range -1 to 1 for this keybinding. On keyboard keys this is always 0 or 1, but for joysticks this can be anywhere in the range -1.0 to 1.0
    
    static void loadKeybindings(const string& filename);
    static void saveKeybindings(const string& filename);
private:
    string name;
    string label;

    std::vector<int> key_number;

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
    
    static void handleEvent(const SDL_Event& event);
    static void updateKeys(int key_number, float value);
    
    static PList<Keybinding> keybindings;
    
    static constexpr int type_mask = (0x0f << 16);
    static constexpr int keyboard_mask = (0x01 << 16);
    static constexpr int pointer_mask =  (0x02 << 16);
    static constexpr int joystick_button_mask = (0x03 << 16);
    static constexpr int joystick_axis_mask = (0x04 << 16);
    
    friend class sp::Engine;
};

};//namespace io
};//namespace sp

#endif//SP2_IO_KEYBINDING_H
