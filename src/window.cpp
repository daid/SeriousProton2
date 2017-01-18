#include <sp2/window.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <sp2/graphics/gui/graphicslayer.h>
#include <SFML/Window/Event.hpp>

namespace sp {

P<Window> Window::window;

Window::Window()
: Window(0.0)
{
}

Window::Window(float aspect_ratio)
{
    sp2assert(!window, "SP2 does not support more then 1 window.");
    window = this;
    antialiasing = 0;
    fullscreen = false;
    this->aspect_ratio = aspect_ratio;
    mouse_button_down_mask = 0;
    
    createRenderWindow();
}

Window::~Window()
{
}

void Window::createRenderWindow()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::ContextSettings context_settings(24, 8, antialiasing, 2, 0);

    float window_width = desktop.width;
    float window_height = desktop.height;
    if (!fullscreen)
    {
        while(window_width >= int(desktop.width) || window_height >= int(desktop.height) - 100)
        {
            window_width *= 0.9;
            window_height *= 0.9;
        }
        if (aspect_ratio != 0.0)
        {
            window_width = window_height * aspect_ratio;
        }
    }

    if (fullscreen)
        render_window.create(sf::VideoMode(window_width, window_height, 32), title, sf::Style::Fullscreen, context_settings);
    else
        render_window.create(sf::VideoMode(window_width, window_height, 32), title, sf::Style::Default, context_settings);

    sf::ContextSettings settings = render_window.getSettings();
    LOG(Info, "OpenGL version:", settings.majorVersion, settings.minorVersion);
    render_window.setVerticalSyncEnabled(false);
    render_window.setFramerateLimit(60);
    render_window.setMouseCursorVisible(true);
}

void Window::render()
{
    render_window.clear(sf::Color(20, 20, 20));
    render_window.setActive();
    for(GraphicsLayer* layer : GraphicsLayer::layers)
    {
        if (layer->isEnabled())
            layer->render(render_window);
    }
    render_window.display();
}

void Window::handleEvent(const sf::Event& event)
{
    switch(event.type)
    {
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
            mouse_button_down_mask |= 1 << int(event.mouseButton.button);
            pointerDown(button, sf::Vector2f(event.mouseButton.x, event.mouseButton.y), -1);
        }
        break;
    case sf::Event::MouseMoved:
        if (mouse_button_down_mask)
            pointerDrag(sf::Vector2f(event.mouseMove.x, event.mouseMove.y), -1);
        break;
    case sf::Event::MouseButtonReleased:
        mouse_button_down_mask &=~(1 << int(event.mouseButton.button));
        if (!mouse_button_down_mask)
            pointerUp(sf::Vector2f(event.mouseButton.x, event.mouseButton.y), -1);
        break;
    case sf::Event::TouchBegan:
        pointerDown(io::Pointer::Button::Touch, sf::Vector2f(event.touch.x, event.touch.y), event.touch.finger);
        break;
    case sf::Event::TouchMoved:
        pointerDrag(sf::Vector2f(event.touch.x, event.touch.y), event.touch.finger);
        break;
    case sf::Event::TouchEnded:
        pointerUp(sf::Vector2f(event.touch.x, event.touch.y), event.touch.finger);
        break;
    default:
        break;
    }
}

void Window::pointerDown(io::Pointer::Button button, sf::Vector2f position, int id)
{
    for(auto l : GraphicsLayer::layers)
    {
        if (l->onPointerDown(button, sf::Vector2f(position.x / float(Window::window->render_window.getSize().x), position.y / float(Window::window->render_window.getSize().y)), id))
        {
            pointer_focus_layer[id] = l;
            return;
        }
    }
}

void Window::pointerDrag(sf::Vector2f position, int id)
{
    auto it = pointer_focus_layer.find(id);
    if (it != pointer_focus_layer.end() && it->second)
        it->second->onPointerDrag(sf::Vector2f(position.x / float(Window::window->render_window.getSize().x), position.y / float(Window::window->render_window.getSize().y)), id);
}

void Window::pointerUp(sf::Vector2f position, int id)
{
    auto it = pointer_focus_layer.find(id);
    if (it != pointer_focus_layer.end() && it->second)
    {
        it->second->onPointerUp(sf::Vector2f(position.x / float(Window::window->render_window.getSize().x), position.y / float(Window::window->render_window.getSize().y)), id);
        pointer_focus_layer.erase(it);
    }
}

};//!namespace sp
