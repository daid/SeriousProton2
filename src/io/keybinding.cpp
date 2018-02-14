#include <sp2/io/keybinding.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/attributes.h>
#include <sp2/engine.h>
#include <json11/json11.hpp>
#include <fstream>

namespace sp {
namespace io {

PList<Keybinding> Keybinding::keybindings SP2_INIT_EARLY;

Keybinding::Keybinding(string name, sf::Keyboard::Key default_key)
: name(name), label(name)
{
    key = default_key;
    pointer_button = Pointer::Button::Unknown;
    
    joystick_index = -1;
    joystick_button_index = -1;
    joystick_axis_enabled = false;
    joystick_axis = sf::Joystick::X;
    joystick_axis_positive = true;
    
    value = 0.0;
    down_event = false;
    up_event = false;
    fixed_down_event = false;
    fixed_up_event = false;
    
    for(Keybinding* other : keybindings)
        sp2assert(other->name != name, "Duplicate keybinding name");
    keybindings.add(this);
}

void Keybinding::setKey(sf::Keyboard::Key key)
{
    this->key = key;
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
        if (entry["key"].is_number())
            keybinding->key = sf::Keyboard::Key(entry["key"].int_value());
        else
            keybinding->key = sf::Keyboard::Unknown;
        if (entry["pointer"].is_number())
            keybinding->pointer_button = Pointer::Button(entry["pointer"].int_value());
        else
            keybinding->pointer_button = Pointer::Button::Unknown;
        if (entry["joystick"].is_number())
        {
            keybinding->joystick_index = entry["joystick"].int_value();
            if (entry["joystick_button"].is_number())
                keybinding->joystick_button_index = entry["joystick_button"].int_value();
            else
                keybinding->joystick_button_index = -1;
            if (entry["joystick_axis"].is_number())
            {
                keybinding->joystick_axis_enabled = true;
                keybinding->joystick_axis = sf::Joystick::Axis(entry["joystick_axis"].int_value());
                keybinding->joystick_axis_positive = entry["joystick_axis_positive"].bool_value();
            }
            else
            {
                keybinding->joystick_axis_enabled = false;
            }
        }
        else
        {
            keybinding->joystick_index = -1;
        }
    }
}

void Keybinding::saveKeybindings(const string& filename)
{
    json11::Json::object obj;
    for(Keybinding* keybinding : keybindings)
    {
        json11::Json::object data;
        if (keybinding->key != sf::Keyboard::Unknown)
            data["key"] = keybinding->key;
        if (keybinding->pointer_button != Pointer::Button::Unknown)
            data["pointer"] = int(keybinding->pointer_button);
        if (keybinding->joystick_index != -1)
        {
            data["joystick"] = keybinding->joystick_index;
            if (keybinding->joystick_button_index != -1)
                data["joystick_button"] = keybinding->joystick_button_index;
            if (keybinding->joystick_axis_enabled)
            {
                data["joystick_axis"] = keybinding->joystick_axis;
                data["joystick_axis_positive"] = keybinding->joystick_axis_positive;
            }
        }
        obj[keybinding->name] = data;
    }
    json11::Json json = obj;
    
    std::ofstream file(filename);
    file << json.dump();
}

void Keybinding::setValue(float value)
{
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

void Keybinding::handleEvent(const sf::Event& event)
{
    switch(event.type)
    {
    case sf::Event::KeyPressed:
        for(Keybinding* key : keybindings)
            if (key->key == event.key.code)
                key->setValue(1.0);
        break;
    case sf::Event::KeyReleased:
        for(Keybinding* key : keybindings)
            if (key->key == event.key.code)
                key->setValue(0.0);
        break;
    case sf::Event::MouseButtonPressed:
        {
            io::Pointer::Button button = io::Pointer::Button::Unknown;
            switch(event.mouseButton.button)
            {
            case sf::Mouse::Left: button = io::Pointer::Button::Left; break;
            case sf::Mouse::Middle: button = io::Pointer::Button::Middle; break;
            case sf::Mouse::Right: button = io::Pointer::Button::Right; break;
            default: break;
            }
            if (button != io::Pointer::Button::Unknown)
                for(Keybinding* key : keybindings)
                    if (key->pointer_button == button || key->pointer_button == io::Pointer::Button::Any)
                            key->setValue(1.0);
        }
        break;
    case sf::Event::MouseButtonReleased:
        {
            io::Pointer::Button button = io::Pointer::Button::Unknown;
            switch(event.mouseButton.button)
            {
            case sf::Mouse::Left: button = io::Pointer::Button::Left; break;
            case sf::Mouse::Middle: button = io::Pointer::Button::Middle; break;
            case sf::Mouse::Right: button = io::Pointer::Button::Right; break;
            default: break;
            }
            if (button != io::Pointer::Button::Unknown)
                for(Keybinding* key : keybindings)
                    if (key->pointer_button == button || key->pointer_button == io::Pointer::Button::Any)
                        key->setValue(0.0);
        }
        break;
    case sf::Event::MouseWheelScrolled:
        break;
    case sf::Event::TouchBegan:
        for(Keybinding* key : keybindings)
            if (key->pointer_button == io::Pointer::Button::Touch || key->pointer_button == io::Pointer::Button::Any)
                key->setValue(1.0);
        break;
    case sf::Event::TouchEnded:
        for(Keybinding* key : keybindings)
            if (key->pointer_button == io::Pointer::Button::Touch || key->pointer_button == io::Pointer::Button::Any)
                key->setValue(0.0);
        break;
    case sf::Event::JoystickButtonPressed:
        for(Keybinding* key : keybindings)
            if (key->joystick_index == int(event.joystickButton.joystickId) && key->joystick_button_index == int(event.joystickButton.button))
                key->setValue(1.0);
        break;
    case sf::Event::JoystickButtonReleased:
        for(Keybinding* key : keybindings)
            if (key->joystick_index == int(event.joystickButton.joystickId) && key->joystick_button_index == int(event.joystickButton.button))
                key->setValue(0.0);
        break;
    case sf::Event::JoystickMoved:
        for(Keybinding* key : keybindings)
            if (key->joystick_axis_enabled && key->joystick_index == int(event.joystickMove.joystickId) && key->joystick_axis == event.joystickMove.axis)
                key->setValue(event.joystickMove.position);
        break;
    case sf::Event::JoystickDisconnected:
        for(Keybinding* key : keybindings)
            if (key->joystick_index == int(event.joystickButton.joystickId))
                key->setValue(0.0);
        break;
    case sf::Event::LostFocus:
        //Focus lost, release all keyboard keys.
        for(Keybinding* key : keybindings)
            if (key->key != sf::Keyboard::Unknown)
                key->setValue(0.0);
        break;
    default:
        break;
    }
}

};//!namespace io
};//!namespace sp
