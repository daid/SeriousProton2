#ifndef SP2_WINDOW_H
#define SP2_WINDOW_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <sp2/pointer.h>
#include <sp2/string.h>

namespace sp {
namespace io { class Clipboard; }

class Window : public AutoPointerObject
{
public:
    sf::RenderWindow render_window;

    Window();
private:
    virtual ~Window();
    
    void createRenderWindow();
    void render();
    
    string title;
    int antialiasing;
    bool fullscreen;

    static P<Window> window;
    
    //Engine needs to be able to access the render window, as this is needed for event polling.
    friend class Engine;
    friend class io::Clipboard;
};

};//!namespace sp

#endif//SP2_WINDOW_H
