#include <sp2/io/keybinding.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/attributes.h>
#include <sp2/engine.h>
#include <sp2/stringutil/convert.h>
#include <json11/json11.hpp>
#include <fstream>
#include <SDL_events.h>

namespace sp {
namespace io {

PList<Keybinding> Keybinding::keybindings SP2_INIT_EARLY;
P<Keybinding> Keybinding::rebinding_key SP2_INIT_EARLY;

Keybinding::Keybinding(string name)
: name(name), label(name)
{
    value = 0.0;
    down_event = false;
    up_event = false;
    fixed_down_event = false;
    fixed_up_event = false;

    for(P<Keybinding> other : keybindings)
        sp2assert(other->name != name, "Duplicate keybinding name");
    keybindings.add(this);
}

Keybinding::Keybinding(string name, string default_key)
: Keybinding(name)
{
    addKey(default_key);
}

Keybinding::Keybinding(string name, const std::initializer_list<const string>& default_keys)
: Keybinding(name)
{
    for(const string& key : default_keys)
        addKey(key);
}

void Keybinding::setKey(string key)
{
    key_number.clear();
    addKey(key);
}

void Keybinding::setKeys(const std::initializer_list<const string>& keys)
{
    key_number.clear();
    for(const string& key : keys)
        addKey(key);
}

void Keybinding::addKey(string key)
{
    //Format for joystick keys:
    //joy:[joystick_id]:axis:[axis_id]
    //joy:[joystick_id]:button:[button_id]
    if (key.startswith("joy:"))
    {
        std::vector<string> parts = key.split(":");
        if (parts.size() == 4)
        {
            int joystick_id = stringutil::convert::toInt(parts[1]);
            int axis_button_id = stringutil::convert::toInt(parts[3]);
            if (parts[2] == "axis")
                key_number.push_back(int(axis_button_id) | int(joystick_id) << 8 | joystick_axis_mask);
            else if (parts[2] == "invertedaxis")
                key_number.push_back(int(axis_button_id) | int(joystick_id) << 8 | joystick_axis_inverted_mask);
            else if (parts[2] == "button")
                key_number.push_back(int(axis_button_id) | int(joystick_id) << 8 | joystick_button_mask);
            else
                LOG(Warning, "Unknown joystick binding:", key);
        }
        return;
    }
    //Format for gamecontroller keys:
    //gamecontroller:[joystick_id]:axis:[axis_name]
    //gamecontroller:[joystick_id]:button:[button_name]
    if (key.startswith("gamecontroller:"))
    {
        std::vector<string> parts = key.split(":");
        if (parts.size() == 4)
        {
            int controller_id = stringutil::convert::toInt(parts[1]);
            if (parts[2] == "axis")
            {
                int axis = SDL_GameControllerGetAxisFromString(parts[3].c_str());
                if (axis < 0)
                {
                    LOG(Warning, "Unknown axis in game controller binding:", key);
                    return;
                }
                key_number.push_back(axis | int(controller_id) << 8 | game_controller_axis_mask);
            }
            else if (parts[2] == "invertedaxis")
            {
                int axis = SDL_GameControllerGetAxisFromString(parts[3].c_str());
                if (axis < 0)
                {
                    LOG(Warning, "Unknown axis in game controller binding:", key);
                    return;
                }
                key_number.push_back(axis | int(controller_id) << 8 | game_controller_axis_inverted_mask);
            }
            else if (parts[2] == "button")
            {
                int button = SDL_GameControllerGetButtonFromString(parts[3].c_str());
                if (button < 0)
                {
                    LOG(Warning, "Unknown button in game controller binding:", key);
                    return;
                }
                key_number.push_back(button | int(controller_id) << 8 | game_controller_button_mask);
            }
            else
            {
                LOG(Warning, "Unknown game controller binding:", key);
            }
        }
        return;
    }
    if (key.startswith("pointer:"))
    {
        key_number.push_back(pointer_mask | stringutil::convert::toInt(key.substr(8)));
        return;
    }
    if (key.startswith("wheel:"))
    {
        if (key == "wheel:x+") key_number.push_back(mouse_wheel_mask | 0);
        else if (key == "wheel:x-") key_number.push_back(mouse_wheel_mask | 1);
        else if (key == "wheel:y+") key_number.push_back(mouse_wheel_mask | 2);
        else if (key == "wheel:y-") key_number.push_back(mouse_wheel_mask | 3);
        else LOG(Warning, "Unknown mouse wheel binding:", key);
        return;
    }
    if (key.startswith("virtual:"))
    {
        int index = stringutil::convert::toInt(key.substr(8));
        key_number.push_back(virtual_mask | index);
        return;
    }

    SDL_Keycode code = SDL_GetKeyFromName(key.c_str());
    if (code != SDLK_UNKNOWN)
        key_number.push_back(code | keyboard_mask);
    else
        LOG(Warning, "Unknown key binding:", key);
}

void Keybinding::clearKeys()
{
    key_number.clear();
}

bool Keybinding::isBound()
{
    return key_number.size() > 0;
}

string Keybinding::getKey(int index)
{
    if (index >= 0 && index < int(key_number.size()))
    {
        int key = key_number[index];
        switch(key & type_mask)
        {
        case keyboard_mask:
            return SDL_GetKeyName(key & ~type_mask);
        case pointer_mask:
            return "pointer:" + string(key & ~type_mask);
        case joystick_axis_mask:
            return "joy:" + string((key >> 8) & 0xff) + ":axis:" + string(key & 0xff);
        case joystick_axis_inverted_mask:
            return "joy:" + string((key >> 8) & 0xff) + ":invertedaxis:" + string(key & 0xff);
        case joystick_button_mask:
            return "joy:" + string((key >> 8) & 0xff) + ":button:" + string(key & 0xff);
        case mouse_wheel_mask:
            switch(key & ~type_mask)
            {
            case 0: return "wheel:x+";
            case 1: return "wheel:x-";
            case 2: return "wheel:y+";
            case 3: return "wheel:y-";
            }
        case game_controller_button_mask:
            return "gamecontroller:" + string((key >> 8) & 0xff) + ":button:" + string(SDL_GameControllerGetStringForButton(SDL_GameControllerButton(key & 0xff)));
        case game_controller_axis_mask:
            return "gamecontroller:" + string((key >> 8) & 0xff) + ":axis:" + string(SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis(key & 0xff)));
        case game_controller_axis_inverted_mask:
            return "gamecontroller:" + string((key >> 8) & 0xff) + ":invertedaxis:" + string(SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis(key & 0xff)));
        case virtual_mask:
            return "virtual:" + string(key & 0xff);
        }
    }
    return "Unknown";
}

bool Keybinding::get() const
{
    if (Engine::getInstance()->isInFixedUpdate())
        return value > 0.5 || fixed_down_event;
    return value > 0.5 || down_event;
}

bool Keybinding::getDown() const
{
    if (Engine::getInstance()->isInFixedUpdate())
        return fixed_down_event;
    return down_event;
}

bool Keybinding::getUp() const
{
    if (Engine::getInstance()->isInFixedUpdate())
        return !fixed_down_event && fixed_up_event;
    return !down_event && up_event;
}

float Keybinding::getValue() const
{
    return value;
}

void Keybinding::startUserRebind()
{
    rebinding_key = this;
}

bool Keybinding::isUserRebinding()
{
    return rebinding_key == this;
}

void Keybinding::loadKeybindings(const string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return;
    std::stringstream data;
    data << file.rdbuf();
    string err;

    json11::Json json = json11::Json::parse(data.str(), err);
    if (err != "")
    {
        LOG(Warning, "Failed to load keybindings from", filename, ":", err);
        return;
    }

    for(P<Keybinding> keybinding : keybindings)
    {
        const json11::Json& entry = json[keybinding->name];
        if (!entry.is_object())
            continue;
        if (entry["key"].is_string())
        {
            keybinding->setKey(entry["key"].string_value());
        }
        else if (entry["key"].is_array())
        {
            keybinding->clearKeys();
            for(auto key_entry : entry["key"].array_items())
            {
                if (key_entry.is_string())
                    keybinding->addKey(key_entry.string_value());
            }
        }
        else
        {
            keybinding->key_number.clear();
        }
    }
}

void Keybinding::saveKeybindings(const string& filename)
{
    json11::Json::object obj;
    for(P<Keybinding> keybinding : keybindings)
    {
        json11::Json::object data;
        json11::Json::array keys;
        for(unsigned int index=0; index<keybinding->key_number.size(); index++)
            keys.push_back(keybinding->getKey(index).c_str());
        data["key"] = keys;
        obj[keybinding->name] = data;
    }
    json11::Json json = obj;

    std::ofstream file(filename);
    file << json.dump();
}

void Keybinding::setVirtualKey(int index, float value)
{
    sp2assert(index >= 0 && index <= 255, "Virtual key indexes need to be in the range 0-255");
    
    updateKeys(virtual_mask | index, value);
}

void Keybinding::setValue(float value)
{
    if (value < 0.01)//Add a tiny dead zone by default. Assists in gamepads that give off "almost zero" in neutral.
        value = 0.0;
    if (this->value < 0.5 && value >= 0.5)
        down_event = fixed_down_event = true;
    if (this->value >= 0.5 && value < 0.5)
        up_event = fixed_up_event = true;
    this->value = value;
}

void Keybinding::postUpdate()
{
    if (down_event)
        down_event = false;
    else if (up_event)
        up_event = false;
}

void Keybinding::postFixedUpdate()
{
    if (fixed_down_event)
        fixed_down_event = false;
    else if (fixed_up_event)
        fixed_up_event = false;
}

void Keybinding::allPostUpdate()
{
    for(P<Keybinding> key : keybindings)
        key->postUpdate();
}

void Keybinding::allPostFixedUpdate()
{
    for(P<Keybinding> key : keybindings)
        key->postFixedUpdate();
}

void Keybinding::handleEvent(const SDL_Event& event)
{
    switch(event.type)
    {
    case SDL_KEYDOWN:
        updateKeys(event.key.keysym.sym | keyboard_mask, 1.0);
        break;
    case SDL_KEYUP:
        updateKeys(event.key.keysym.sym | keyboard_mask, 0.0);
        break;
    case SDL_MOUSEBUTTONDOWN:
        {
            io::Pointer::Button button = io::Pointer::Button::Unknown;
            switch(event.button.button)
            {
            case SDL_BUTTON_LEFT: button = io::Pointer::Button::Left; break;
            case SDL_BUTTON_MIDDLE: button = io::Pointer::Button::Middle; break;
            case SDL_BUTTON_RIGHT: button = io::Pointer::Button::Right; break;
            case SDL_BUTTON_X1: button = io::Pointer::Button::Other1; break;
            case SDL_BUTTON_X2: button = io::Pointer::Button::Other2; break;
            default: break;
            }
            if (button != io::Pointer::Button::Unknown)
                updateKeys(int(button) | pointer_mask, 1.0);
        }
        break;
    case SDL_MOUSEBUTTONUP:
        {
            io::Pointer::Button button = io::Pointer::Button::Unknown;
            switch(event.button.button)
            {
            case SDL_BUTTON_LEFT: button = io::Pointer::Button::Left; break;
            case SDL_BUTTON_MIDDLE: button = io::Pointer::Button::Middle; break;
            case SDL_BUTTON_RIGHT: button = io::Pointer::Button::Right; break;
            case SDL_BUTTON_X1: button = io::Pointer::Button::Other1; break;
            case SDL_BUTTON_X2: button = io::Pointer::Button::Other2; break;
            default: break;
            }
            if (button != io::Pointer::Button::Unknown)
                updateKeys(int(button) | pointer_mask, 0.0);
        }
        break;
    case SDL_MOUSEWHEEL:
        if (event.wheel.x > 0)
        {
            updateKeys(0 | mouse_wheel_mask, 1.0);
            updateKeys(0 | mouse_wheel_mask, 0.0);
        }
        if (event.wheel.x < 0)
        {
            updateKeys(1 | mouse_wheel_mask, 1.0);
            updateKeys(1 | mouse_wheel_mask, 0.0);
        }
        if (event.wheel.y > 0)
        {
            updateKeys(2 | mouse_wheel_mask, 1.0);
            updateKeys(2 | mouse_wheel_mask, 0.0);
        }
        if (event.wheel.y < 0)
        {
            updateKeys(3 | mouse_wheel_mask, 1.0);
            updateKeys(3 | mouse_wheel_mask, 0.0);
        }
        break;
    case SDL_FINGERDOWN:
        //event.tfinger.x, event.tfinger.x
        updateKeys(int(io::Pointer::Button::Touch) | pointer_mask, 1.0);
        break;
    case SDL_FINGERUP:
        //event.tfinger.x, event.tfinger.x
        updateKeys(int(io::Pointer::Button::Touch) | pointer_mask, 0.0);
        break;
    case SDL_JOYBUTTONDOWN:
        updateKeys(int(event.jbutton.button) | int(event.jbutton.which) << 8 | joystick_button_mask, 1.0);
        break;
    case SDL_JOYBUTTONUP:
        updateKeys(int(event.jbutton.button) | int(event.jbutton.which) << 8 | joystick_button_mask, 0.0);
        break;
    case SDL_JOYAXISMOTION:
        updateKeys(int(event.jaxis.axis) | int(event.jaxis.which) << 8 | joystick_axis_mask, float(event.jaxis.value) / 32768.0);
        updateKeys(int(event.jaxis.axis) | int(event.jaxis.which) << 8 | joystick_axis_inverted_mask, -float(event.jaxis.value) / 32768.0);
        break;
    case SDL_JOYDEVICEADDED:
        if (!SDL_IsGameController(event.jdevice.which))
        {
            SDL_Joystick* joystick = SDL_JoystickOpen(event.jdevice.which);
            if (joystick)
                LOG(Info, "Found joystick:", SDL_JoystickName(joystick));
            else
                LOG(Warning, "Failed to open joystick...");
        }
        break;
    case SDL_JOYDEVICEREMOVED:
        for(int button=0; button<32; button++)
            updateKeys(int(button) | int(event.jdevice.which) << 8 | joystick_button_mask, 0.0);
        for(int axis=0; axis<32; axis++)
        {
            updateKeys(int(axis) | int(event.jdevice.which) << 8 | joystick_axis_mask, 0.0);
            updateKeys(int(axis) | int(event.jdevice.which) << 8 | joystick_axis_inverted_mask, 0.0);
        }
        SDL_JoystickClose(SDL_JoystickFromInstanceID(event.jdevice.which));
        break;
    case SDL_CONTROLLERAXISMOTION:
        updateKeys(int(event.caxis.axis) | int(event.caxis.which) << 8 | game_controller_axis_mask, float(event.caxis.value) / 32768.0);
        updateKeys(int(event.caxis.axis) | int(event.caxis.which) << 8 | game_controller_axis_inverted_mask, -float(event.caxis.value) / 32768.0);
        break;
    case SDL_CONTROLLERBUTTONDOWN:
        updateKeys(int(event.cbutton.button) | int(event.cbutton.which) << 8 | game_controller_button_mask, 1.0);
        break;
    case SDL_CONTROLLERBUTTONUP:
        updateKeys(int(event.cbutton.button) | int(event.cbutton.which) << 8 | game_controller_button_mask, 0.0);
        break;
    case SDL_CONTROLLERDEVICEADDED:
        {
            SDL_GameController* gc = SDL_GameControllerOpen(event.cdevice.which);
            if (gc)
                LOG(Info, "Found game controller:", SDL_GameControllerName(gc));
            else
                LOG(Warning, "Failed to open game controller...");
        }
        break;
    case SDL_CONTROLLERDEVICEREMOVED:
        for(int button=0; button<SDL_CONTROLLER_BUTTON_MAX; button++)
            updateKeys(int(button) | int(event.cdevice.which) << 8 | game_controller_button_mask, 0.0);
        for(int axis=0; axis<SDL_CONTROLLER_AXIS_MAX; axis++)
        {
            updateKeys(int(axis) | int(event.cdevice.which) << 8 | game_controller_axis_mask, 0.0);
            updateKeys(int(axis) | int(event.cdevice.which) << 8 | game_controller_axis_inverted_mask, 0.0);
        }
        SDL_GameControllerClose(SDL_GameControllerFromInstanceID(event.cdevice.which));
        break;
    case SDL_CONTROLLERDEVICEREMAPPED:
        break;
    case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
            //Focus lost, release all keys.
            for(P<Keybinding> key : keybindings)
                if (key->key_number.size() > 0)
                    key->setValue(0.0);
        }
        break;
    default:
        break;
    }
}

void Keybinding::updateKeys(int key_number, float value)
{
    if (value > 0.5 && rebinding_key)
    {
        rebinding_key->key_number.push_back(key_number);
        rebinding_key = nullptr;
    }

    for(P<Keybinding> key : keybindings)
        for(int key_nr : key->key_number)
            if (key_nr == key_number)
                key->setValue(value);
}

};//namespace io
};//namespace sp
