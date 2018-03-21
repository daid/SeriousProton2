#include <sp2/io/keybinding.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/attributes.h>
#include <sp2/engine.h>
#include <json11/json11.hpp>
#include <fstream>

namespace sp {
namespace io {

static std::unordered_map<string, sf::Keyboard::Key> sfml_key_names SP2_INIT_EARLY = {
    {"A", sf::Keyboard::A},
    {"B", sf::Keyboard::B},
    {"C", sf::Keyboard::C},
    {"D", sf::Keyboard::D},
    {"E", sf::Keyboard::E},
    {"F", sf::Keyboard::F},
    {"G", sf::Keyboard::G},
    {"H", sf::Keyboard::H},
    {"I", sf::Keyboard::I},
    {"J", sf::Keyboard::J},
    {"K", sf::Keyboard::K},
    {"L", sf::Keyboard::L},
    {"M", sf::Keyboard::M},
    {"N", sf::Keyboard::N},
    {"O", sf::Keyboard::O},
    {"P", sf::Keyboard::P},
    {"Q", sf::Keyboard::Q},
    {"R", sf::Keyboard::R},
    {"S", sf::Keyboard::S},
    {"T", sf::Keyboard::T},
    {"U", sf::Keyboard::U},
    {"V", sf::Keyboard::V},
    {"W", sf::Keyboard::W},
    {"X", sf::Keyboard::X},
    {"Y", sf::Keyboard::Y},
    {"Z", sf::Keyboard::Z},
    {"Num0", sf::Keyboard::Num0},
    {"Num1", sf::Keyboard::Num1},
    {"Num2", sf::Keyboard::Num2},
    {"Num3", sf::Keyboard::Num3},
    {"Num4", sf::Keyboard::Num4},
    {"Num5", sf::Keyboard::Num5},
    {"Num6", sf::Keyboard::Num6},
    {"Num7", sf::Keyboard::Num7},
    {"Num8", sf::Keyboard::Num8},
    {"Num9", sf::Keyboard::Num9},
    {"Escape", sf::Keyboard::Escape},
    {"LControl", sf::Keyboard::LControl},
    {"LShift", sf::Keyboard::LShift},
    {"LAlt", sf::Keyboard::LAlt},
    {"LSystem", sf::Keyboard::LSystem},
    {"RControl", sf::Keyboard::RControl},
    {"RShift", sf::Keyboard::RShift},
    {"RAlt", sf::Keyboard::RAlt},
    {"RSystem", sf::Keyboard::RSystem},
    {"Menu", sf::Keyboard::Menu},
    {"LBracket", sf::Keyboard::LBracket},
    {"RBracket", sf::Keyboard::RBracket},
    {"SemiColon", sf::Keyboard::SemiColon},
    {"Comma", sf::Keyboard::Comma},
    {"Period", sf::Keyboard::Period},
    {"Quote", sf::Keyboard::Quote},
    {"Slash", sf::Keyboard::Slash},
    {"BackSlash", sf::Keyboard::BackSlash},
    {"Tilde", sf::Keyboard::Tilde},
    {"Equal", sf::Keyboard::Equal},
    {"Dash", sf::Keyboard::Dash},
    {"Space", sf::Keyboard::Space},
    {"Return", sf::Keyboard::Return},
    {"BackSpace", sf::Keyboard::BackSpace},
    {"Tab", sf::Keyboard::Tab},
    {"PageUp", sf::Keyboard::PageUp},
    {"PageDown", sf::Keyboard::PageDown},
    {"End", sf::Keyboard::End},
    {"Home", sf::Keyboard::Home},
    {"Insert", sf::Keyboard::Insert},
    {"Delete", sf::Keyboard::Delete},
    {"Add", sf::Keyboard::Add},
    {"Subtract", sf::Keyboard::Subtract},
    {"Multiply", sf::Keyboard::Multiply},
    {"Divide", sf::Keyboard::Divide},
    {"Left", sf::Keyboard::Left},
    {"Right", sf::Keyboard::Right},
    {"Up", sf::Keyboard::Up},
    {"Down", sf::Keyboard::Down},
    {"Numpad0", sf::Keyboard::Numpad0},
    {"Numpad1", sf::Keyboard::Numpad1},
    {"Numpad2", sf::Keyboard::Numpad2},
    {"Numpad3", sf::Keyboard::Numpad3},
    {"Numpad4", sf::Keyboard::Numpad4},
    {"Numpad5", sf::Keyboard::Numpad5},
    {"Numpad6", sf::Keyboard::Numpad6},
    {"Numpad7", sf::Keyboard::Numpad7},
    {"Numpad8", sf::Keyboard::Numpad8},
    {"Numpad9", sf::Keyboard::Numpad9},
    {"F1", sf::Keyboard::F1},
    {"F2", sf::Keyboard::F2},
    {"F3", sf::Keyboard::F3},
    {"F4", sf::Keyboard::F4},
    {"F5", sf::Keyboard::F5},
    {"F6", sf::Keyboard::F6},
    {"F7", sf::Keyboard::F7},
    {"F8", sf::Keyboard::F8},
    {"F9", sf::Keyboard::F9},
    {"F10", sf::Keyboard::F10},
    {"F11", sf::Keyboard::F11},
    {"F12", sf::Keyboard::F12},
    {"F13", sf::Keyboard::F13},
    {"F14", sf::Keyboard::F14},
    {"F15", sf::Keyboard::F15},
    {"Pause", sf::Keyboard::Pause},
};

PList<Keybinding> Keybinding::keybindings SP2_INIT_EARLY;

Keybinding::Keybinding(string name, string default_key)
: name(name), label(name)
{
    setKey(default_key);
    
    value = 0.0;
    down_event = false;
    up_event = false;
    fixed_down_event = false;
    fixed_up_event = false;
    
    for(Keybinding* other : keybindings)
        sp2assert(other->name != name, "Duplicate keybinding name");
    keybindings.add(this);
}

void Keybinding::setKey(string key)
{
    auto it = sfml_key_names.find(key);
    if (it == sfml_key_names.end())
        this->key_number = -1;
    else
        this->key_number = (it->second | keyboard_mask);
}

string Keybinding::getKey()
{
    for(auto it : sfml_key_names)
    {
        if (it.second == (key_number | keyboard_mask))
            return it.first;
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
            keybinding->setKey(entry["key"].string_value());
        else
            keybinding->key_number = -1;
    }
}

void Keybinding::saveKeybindings(const string& filename)
{
    json11::Json::object obj;
    for(Keybinding* keybinding : keybindings)
    {
        json11::Json::object data;
        if (keybinding->key_number != -1)
            data["key"] = keybinding->getKey();
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
        updateKeys(event.key.code | keyboard_mask, 1.0);
        break;
    case sf::Event::KeyReleased:
        updateKeys(event.key.code | keyboard_mask, 0.0);
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
                updateKeys(int(button) | pointer_mask, 1.0);
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
                updateKeys(int(button) | pointer_mask, 0.0);
        }
        break;
    case sf::Event::MouseWheelScrolled:
        //event.mouseWheel.wheel
        //event.mouseWheel.delta
        break;
    case sf::Event::TouchBegan:
        updateKeys(int(io::Pointer::Button::Touch) | pointer_mask, 1.0);
        break;
    case sf::Event::TouchEnded:
        updateKeys(int(io::Pointer::Button::Touch) | pointer_mask, 0.0);
        break;
    case sf::Event::JoystickButtonPressed:
        updateKeys(int(event.joystickButton.button) | int(event.joystickButton.joystickId) << 8 | joystick_button_mask, 1.0);
        break;
    case sf::Event::JoystickButtonReleased:
        updateKeys(int(event.joystickButton.button) | int(event.joystickButton.joystickId) << 8 | joystick_button_mask, 0.0);
        break;
    case sf::Event::JoystickMoved:
        updateKeys(int(event.joystickMove.axis) | int(event.joystickMove.joystickId) << 8 | joystick_axis_mask, event.joystickMove.position);
        break;
    case sf::Event::JoystickDisconnected:
        for(int button=0; button<sf::Joystick::ButtonCount; button++)
            updateKeys(int(button) | int(event.joystickConnect.joystickId) << 8 | joystick_button_mask, 0.0);
        for(int axis=0; axis<sf::Joystick::AxisCount; axis++)
            updateKeys(int(axis) | int(event.joystickConnect.joystickId) << 8 | joystick_axis_mask, 0.0);
        break;
    case sf::Event::LostFocus:
        //Focus lost, release all keys.
        for(Keybinding* key : keybindings)
            if (key->key_number != -1)
                key->setValue(0.0);
        break;
    default:
        break;
    }
}

void Keybinding::updateKeys(int key_number, float value)
{
    for(Keybinding* key : keybindings)
        if (key->key_number == key_number)
            key->setValue(value);
}

};//!namespace io
};//!namespace sp
