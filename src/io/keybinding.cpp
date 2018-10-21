#include <sp2/io/keybinding.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/attributes.h>
#include <sp2/engine.h>
#include <sp2/stringutil/convert.h>
#include <json11/json11.hpp>
#include <fstream>

#include <SDL2/SDL_events.h>

namespace sp {
namespace io {

PList<Keybinding> Keybinding::keybindings SP2_INIT_EARLY;

Keybinding::Keybinding(string name)
: name(name), label(name)
{
    value = 0.0;
    down_event = false;
    up_event = false;
    fixed_down_event = false;
    fixed_up_event = false;
    
    for(Keybinding* other : keybindings)
        sp2assert(other->name != name, "Duplicate keybinding name");
    keybindings.add(this);
}

Keybinding::Keybinding(string name, string default_key)
: Keybinding(name)
{
    addKey(default_key);
}

Keybinding::Keybinding(string name, string default_key, string alternative_default_key)
: Keybinding(name)
{
    addKey(default_key);
    addKey(alternative_default_key);
}

void Keybinding::setKey(string key)
{
    key_number.clear();
    addKey(key);
}

void Keybinding::setKeys(string key1, string key2)
{
    setKey(key1);
    addKey(key2);
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
            else if (parts[2] == "button")
                key_number.push_back(int(axis_button_id) | int(joystick_id) << 8 | joystick_button_mask);
            else
                LOG(Warning, "Unknown joystick binding:", key);
        }
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
        case joystick_axis_mask:
            return "joy:" + string((key >> 8) & 0xff) + ":axis:" + string(key & 0xff);
        case joystick_button_mask:
            return "joy:" + string((key >> 8) & 0xff) + ":button:" + string(key & 0xff);
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
    
    for(Keybinding* keybinding : keybindings)
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
    for(Keybinding* keybinding : keybindings)
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

void Keybinding::setValue(float value)
{
    if (std::abs(value) < 0.01)//Add a tiny dead zone by default. Assists in gamepads that give off "almost zero" in neutral.
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
    for(Keybinding* key : keybindings)
        key->postUpdate();
}

void Keybinding::allPostFixedUpdate()
{
    for(Keybinding* key : keybindings)
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
            default: break;
            }
            if (button != io::Pointer::Button::Unknown)
                updateKeys(int(button) | pointer_mask, 0.0);
        }
        break;
    case SDL_MOUSEWHEEL:
        //event.mouseWheel.wheel
        //event.mouseWheel.delta
        break;
    case SDL_FINGERDOWN:
        updateKeys(int(io::Pointer::Button::Touch) | pointer_mask, 1.0);
        break;
    case SDL_FINGERUP:
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
        break;
    case SDL_JOYDEVICEADDED:
        break;
    case SDL_JOYDEVICEREMOVED:
        for(int button=0; button<32; button++)
            updateKeys(int(button) | int(event.jdevice.which) << 8 | joystick_button_mask, 0.0);
        for(int axis=0; axis<32; axis++)
            updateKeys(int(axis) | int(event.jdevice.which) << 8 | joystick_axis_mask, 0.0);
        break;
    case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
            //Focus lost, release all keys.
            for(Keybinding* key : keybindings)
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
    for(Keybinding* key : keybindings)
        for(int key_nr : key->key_number)
            if (key_nr == key_number)
                key->setValue(value);
}

};//namespace io
};//namespace sp
