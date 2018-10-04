#include <sp2/window.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <sp2/graphics/graphicslayer.h>
#include <sp2/graphics/renderTexture.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <sp2/graphics/opengl.h>
#include <SFML/Window/Event.hpp>

#ifdef __WIN32__
#include <Windows.h>
#endif


namespace sp {

PList<Window> Window::windows;

Window::Window()
{
    windows.add(this);
    antialiasing = 0;
    fullscreen = false;
    mouse_button_down_mask = 0;

    window_aspect_ratio = 0.0;
    clear_color = Color(0.1, 0.1, 0.1);
    
    createRenderWindow();
}

Window::Window(float aspect_ratio)
: Window()
{
    window_aspect_ratio = aspect_ratio;
    
    createRenderWindow();
}

Window::Window(Vector2f size_factor)
: Window()
{
    max_window_size_ratio = size_factor;
    
    createRenderWindow();
}

Window::Window(Vector2f size_factor, float aspect_ratio)
: Window()
{
    window_aspect_ratio = aspect_ratio;
    max_window_size_ratio = size_factor;

    createRenderWindow();
}

Window::~Window()
{
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

void Window::setPosition(Vector2f position)
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::Vector2u size = render_window.getSize();

#ifdef __WIN32__
    // Account for window borders. SFML does not have a method to query this.
    RECT rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&rect, GetWindowLong((HWND)render_window.getSystemHandle(), GWL_STYLE), false, GetWindowLong((HWND)render_window.getSystemHandle(), GWL_EXSTYLE));
    size.x += rect.right - rect.left;
    size.y += rect.bottom - rect.top;
    
    // Adjust the desktop size for the taskbar.
    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(monitor_info);
    GetMonitorInfoA(MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY), &monitor_info);
    desktop.width = monitor_info.rcWork.right;
    desktop.height = monitor_info.rcWork.bottom;
#endif//__WIN32__

    render_window.setPosition(sf::Vector2i((desktop.width - size.x) * position.x, (desktop.height - size.y) * position.y));
}

void Window::addLayer(P<GraphicsLayer> layer)
{
    graphics_layers.add(layer);
}

void Window::createRenderWindow()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::ContextSettings context_settings(24, 8, antialiasing, 2, 0);

    float window_width = desktop.width;
    float window_height = desktop.height;
    if (!fullscreen)
    {
        if (max_window_size_ratio.x != 0.0 && max_window_size_ratio.y != 0.0)
        {
            window_width *= max_window_size_ratio.x;
            window_height *= max_window_size_ratio.y;
        }
        else
        {
            //Make sure the window fits on the screen with some edge around it.
            while(window_width >= int(desktop.width) || window_height >= int(desktop.height) - 100)
            {
                window_width *= 0.9;
                window_height *= 0.9;
            }
        }

        if (window_aspect_ratio != 0.0)
        {
            if (window_width > window_height * window_aspect_ratio)
                window_width = window_height * window_aspect_ratio;
            else if (window_height > window_width / window_aspect_ratio)
                window_height = window_width / window_aspect_ratio;
        }
    }

    if (fullscreen)
        render_window.create(sf::VideoMode(window_width, window_height, 32), title, sf::Style::Fullscreen, context_settings);
    else
        render_window.create(sf::VideoMode(window_width, window_height, 32), title, sf::Style::Default, context_settings);
        
    sf::ContextSettings settings = render_window.getSettings();
    LOG(Info, "OpenGL version:", settings.majorVersion, settings.minorVersion);

    render_window.setVerticalSyncEnabled(true);
    //render_window.setVerticalSyncEnabled(false);
    //render_window.setFramerateLimit(60);

    render_window.setMouseCursorVisible(true);
    render_window.setKeyRepeatEnabled(false);
    render_window.setActive(false);
}

void Window::render()
{
    graphics_layers.sort([](const P<GraphicsLayer>& a, const P<GraphicsLayer>& b){
        return a->priority - b->priority;
    });

    queue.add([this]()
    {
        glViewport(0, 0, 1, 1);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    });
    for(GraphicsLayer* layer : graphics_layers)
    {
        if (layer->isEnabled())
        {
            Vector2i size(render_window.getSize().x, render_window.getSize().y);
            if (layer->getTarget())
            {
                size = layer->getTarget()->getSize();
                queue.add([layer]()
                {
                    layer->getTarget()->activateRenderTarget();
                    glViewport(0, 0, layer->getTarget()->getSize().x, layer->getTarget()->getSize().y);
                });
            }
            else
            {
                queue.add([this, layer]()
                {
                    render_window.setActive();
                    Vector2d size(render_window.getSize().x, render_window.getSize().y);
                    Rect2d viewport = layer->viewport;
                    glViewport(viewport.position.x * size.x, viewport.position.y * size.y, viewport.size.x * size.x, viewport.size.y * size.y);
                });
            }
            
            queue.setTargetAspectSize(float(size.x) / float(size.y));
            layer->render(queue);
        }
    }

    if (cursor_mesh && cursor_texture)
    {
        queue.add([this]()
        {
            render_window.setActive();
            glViewport(0, 0, render_window.getSize().x, render_window.getSize().y);
        });
        sf::Vector2i mouse_pos = sf::Mouse::getPosition(render_window);
    
        Vector2d position(mouse_pos.x, mouse_pos.y);
        Vector2d window_size(render_window.getSize().x, render_window.getSize().y);
        position.x = position.x - window_size.x / 2.0;
        position.y = window_size.y / 2.0 - position.y;
        
        queue.setCamera(Matrix4x4d::scale(2.0/window_size.x, 2.0/window_size.y, 1), Matrix4x4d::identity());
        RenderData rd;
        rd.type = RenderData::Type::Normal;
        rd.shader = Shader::get("internal:basic.shader");
        rd.mesh = cursor_mesh;
        rd.texture = cursor_texture;
        queue.add(Matrix4x4d::translate(position.x, position.y, 0), rd);
    }
    queue.add([this]()
    {
        render_window.display();
    });
    
    queue.render();
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
    for(auto l : graphics_layers)
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

bool Window::anyWindowOpen()
{
    for(Window* window : windows)
    {
        if (window->render_window.isOpen())
        {
            return true;
        }
    }
    return false;
}

};//namespace sp
