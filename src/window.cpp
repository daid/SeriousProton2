#include <sp2/window.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <sp2/graphics/graphicslayer.h>
#include <sp2/graphics/scene/renderqueue.h>
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

P<Window> Window::getInstance()
{
    return window;
}

void Window::setFullScreen(bool fullscreen)
{
    this->fullscreen = fullscreen;
    createRenderWindow();
}

void Window::setClearColor(sp::Color color)
{
    clear_color = color;
}

void Window::hideCursor()
{
    render_window.setMouseCursorVisible(false);
    cursor_texture = nullptr;
    cursor_mesh = nullptr;
}

void Window::setDefaultCursor()
{
    render_window.setMouseCursorVisible(true);
    cursor_texture = nullptr;
    cursor_mesh = nullptr;
}

void Window::setCursor(Texture* texture, std::shared_ptr<MeshData> mesh)
{
    render_window.setMouseCursorVisible(false);
    cursor_texture = texture;
    cursor_mesh = mesh;
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
    
    clear_color = Color(0.1, 0.1, 0.1);
}

void Window::render()
{
    render_window.clear(sf::Color(clear_color.toInt()));
    render_window.setActive();
    for(GraphicsLayer* layer : GraphicsLayer::layers)
    {
        if (layer->isEnabled())
            layer->render(render_window);
    }
    
    if (cursor_mesh && cursor_texture)
    {
        sf::Vector2i mouse_pos = sf::Mouse::getPosition(render_window);
    
        Vector2d position(mouse_pos.x, mouse_pos.y);
        Vector2d window_size(render_window.getSize().x, render_window.getSize().y);
        position.x = position.x - window_size.x / 2.0;
        position.y = window_size.y / 2.0 - position.y;
        
        RenderQueue queue;
        RenderData rd;
        rd.type = RenderData::Type::Normal;
        rd.shader = Shader::get("internal:basic.shader");
        rd.mesh = cursor_mesh;
        rd.texture = cursor_texture;
        queue.add(Matrix4x4d::translate(position.x, position.y, 0), rd);
        queue.render(Matrix4x4d::scale(2.0/window_size.x, 2.0/window_size.y, 1), Matrix4x4d::identity());
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
            pointerDown(button, screenToGLPosition(event.mouseButton.x, event.mouseButton.y), -1);
        }
        break;
    case sf::Event::MouseMoved:
        if (mouse_button_down_mask)
            pointerDrag(screenToGLPosition(event.mouseMove.x, event.mouseMove.y), -1);
        break;
    case sf::Event::MouseButtonReleased:
        mouse_button_down_mask &=~(1 << int(event.mouseButton.button));
        if (!mouse_button_down_mask)
            pointerUp(screenToGLPosition(event.mouseButton.x, event.mouseButton.y), -1);
        break;
    case sf::Event::TouchBegan:
        pointerDown(io::Pointer::Button::Touch, screenToGLPosition(event.touch.x, event.touch.y), event.touch.finger);
        break;
    case sf::Event::TouchMoved:
        pointerDrag(screenToGLPosition(event.touch.x, event.touch.y), event.touch.finger);
        break;
    case sf::Event::TouchEnded:
        pointerUp(screenToGLPosition(event.touch.x, event.touch.y), event.touch.finger);
        break;
    default:
        break;
    }
}

void Window::pointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    for(auto l : GraphicsLayer::layers)
    {
        if (l->onPointerDown(button, position, id))
        {
            pointer_focus_layer[id] = l;
            return;
        }
    }
}

void Window::pointerDrag(Vector2d position, int id)
{
    auto it = pointer_focus_layer.find(id);
    if (it != pointer_focus_layer.end() && it->second)
        it->second->onPointerDrag(position, id);
}

void Window::pointerUp(Vector2d position, int id)
{
    auto it = pointer_focus_layer.find(id);
    if (it != pointer_focus_layer.end() && it->second)
    {
        it->second->onPointerUp(position, id);
        pointer_focus_layer.erase(it);
    }
}

Vector2d Window::screenToGLPosition(int x, int y)
{
    sf::Vector2u size = render_window.getSize();
    return Vector2d((double(x) / double(size.x) - 0.5) * 2.0, (0.5 - double(y) / double(size.y)) * 2.0);
}

};//!namespace sp
