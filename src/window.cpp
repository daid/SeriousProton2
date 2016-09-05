#include <sp2/window.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <sp2/graphics/gui/graphicslayer.h>

namespace sp {

P<Window> Window::window;

Window::Window()
{
    sp2assert(!window);
    window = this;
    antialiasing = 0;
    fullscreen = false;
    
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
    }

    if (fullscreen)
        render_window.create(sf::VideoMode(window_width, window_height, 32), title, sf::Style::Fullscreen, context_settings);
    else
        render_window.create(sf::VideoMode(window_width, window_height, 32), title, sf::Style::Default, context_settings);

    sf::ContextSettings settings = render_window.getSettings();
    LOG(Info, "OpenGL version:", settings.majorVersion, '.', settings.minorVersion);
    render_window.setVerticalSyncEnabled(false);
    render_window.setFramerateLimit(60);
    render_window.setMouseCursorVisible(false);
}

void Window::render()
{
    render_window.clear(sf::Color(20, 20, 20));
    for(GraphicsLayer* layer : GraphicsLayer::layers)
    {
        layer->render(render_window);
    }
    render_window.display();
}

};//!namespace sp
