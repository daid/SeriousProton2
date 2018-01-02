#include <sp2/io/keybinding.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/attributes.h>
#include <json11/json11.hpp>
#include <fstream>

namespace sp {
namespace io {

PList<Keybinding> Keybinding::keybindings SP2_INIT_EARLY;

Keybinding::Keybinding(string name, sf::Keyboard::Key default_key)
: name(name), label(name)
{
    key = default_key;
    joystick_index = -1;
    joystick_button_index = -1;
    joystick_axis_enabled = false;
    joystick_axis = sf::Joystick::X;
    joystick_axis_positive = true;
    
    value = 0.0;
    previous_value = 0.0;
    
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
    return value > 0.5;
}

bool Keybinding::getDown() const
{
    return value > 0.5 && previous_value <= 0.5;
}

bool Keybinding::getUp() const
{
    return value <= 0.5 && previous_value > 0.5;
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

void Keybinding::update()
{
    previous_value = value;
    value = 0.0;
    
    if (key != sf::Keyboard::Unknown)
    {
        if (sf::Keyboard::isKeyPressed(key))
            value = 1.0;
    }
    if (joystick_index != -1)
    {
        if (joystick_button_index != -1 && sf::Joystick::isButtonPressed(joystick_index, joystick_button_index))
            value = 1.0;
        if (joystick_axis_enabled)
        {
            float f = sf::Joystick::getAxisPosition(joystick_index, joystick_axis);
            if (!joystick_axis_positive)
                f = -f;
            if (f > 0.0)
                value = f / 100.0f;
        }
    }
}

void Keybinding::updateAll()
{
    for(Keybinding* key : keybindings)
    {
        key->update();
    }
}

};//!namespace io
};//!namespace sp
