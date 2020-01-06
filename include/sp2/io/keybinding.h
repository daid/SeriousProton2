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
    //Create a keybinding, and optionally set the default key(s). See setKey for documentation on key naming.
    Keybinding(const string& name);
    Keybinding(const string& name, const string& default_key);
    Keybinding(const string& name, const std::initializer_list<const string>& default_keys);

    const string& getName() const { return name; }
    const string& getLabel() const { return label; }

    //Set/get/add bound keys to the keybinding.
    //  Key name should be one of the following:
    //  * name of a keyboard key ("Space", "Left", "A")
    //  * "joy:[id]:axis:[axis]" for a joystick axis, where id is the index of the joystick. And axis is the axis number.
    //  * "joy:[id]:invertedaxis:[axis]" for a joystick axis, where id is the index of the joystick. And axis is the axis number.
    //  * "joy:[id]:button:[button]" for a joystick button, where id is the index of the joystick. And axis is the button number.
    //  * "gamecontroller:[id]:axis:[axis], where axis is "leftx", "lefty", "rightx", "righty", "lefttrigger" or "righttrigger"
    //  * "gamecontroller:[id]:invertedaxis:[axis], where axis is "leftx", "lefty", "rightx", "righty", "lefttrigger" or "righttrigger"
    //  * "gamecontroller:[id]:button:[button], where button is one of "a", "b", "x", "y", "back", "guide", "start", "leftstick", "rightstick", "leftshoulder", "rightshoulder", "dpup", "dpdown", "dpleft", "dpright"
    //  * "pointer:[id], where id is 0 to 4 for left,right,middle mouse button, or a touch screen press.
    //  * "wheel:[dir], where dir is one of "x+", "x-", "y+", "y-" for mouse wheel binding. Note that these are never helt down. Only generate up&down events.
    //  * "virtual:[id], where id is the number of a virtual key, controlled by setVirtualKey, and the virtual touchscreen controls.
    void setKey(const string& key);
    void setKeys(const std::initializer_list<const string>& keys);
    string getKey(int index);
    void addKey(const string& key);
    void clearKeys();

    //Returns true if there is anything bound to this keybinding.
    bool isBound();

    bool get() const; //True when this key is currently being pressed.
    bool getDown() const; //True for 1 update cycle when the key is pressed.
    bool getUp() const; //True for 1 update cycle when the key is released.
    float getValue() const; //Returns a value in the range -1 to 1 for this keybinding. On keyboard keys this is always 0 or 1, but for joysticks this can be anywhere in the range -1.0 to 1.0

    //Start a binding process from the user. The next button pressed by the user will be bound to this key.
    //Note that this will add on top of the already existing binds, so clearKeys() need to be called if you want to bind a single key.
    void startUserRebind();
    bool isUserRebinding();

    static int joystickCount();
    static int gamepadCount();

    static void loadKeybindings(const string& filename);
    static void saveKeybindings(const string& filename);

    static P<Keybinding> getByName(const string& name);

    static void setVirtualKey(int index, float value);
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
    static P<Keybinding> rebinding_key;
    
    static constexpr int type_mask = (0x0f << 16);
    static constexpr int keyboard_mask = (0x01 << 16);
    static constexpr int pointer_mask =  (0x02 << 16);
    static constexpr int joystick_button_mask = (0x03 << 16);
    static constexpr int joystick_axis_mask = (0x04 << 16);
    static constexpr int joystick_axis_inverted_mask = (0x05 << 16);
    static constexpr int mouse_wheel_mask = (0x06 << 16);
    static constexpr int game_controller_button_mask = (0x07 << 16);
    static constexpr int game_controller_axis_mask = (0x08 << 16);
    static constexpr int game_controller_axis_inverted_mask = (0x09 << 16);
    static constexpr int virtual_mask = (0x0f << 16);
    
    friend class sp::Engine;
};

}//namespace io
}//namespace sp

#endif//SP2_IO_KEYBINDING_H
