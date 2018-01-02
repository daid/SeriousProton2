#ifndef SP2_WINDOW_H
#define SP2_WINDOW_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/io/pointer.h>
#include <map>

namespace sp {
namespace io { class Clipboard; }

class GraphicsLayer;
class Window : public AutoPointerObject
{
public:
    sf::RenderWindow render_window;

    Window();
    Window(float aspect_ratio);
    
    void setFullScreen(bool fullscreen);
    void setClearColor(sf::Color color);
private:
    virtual ~Window();
    
    void createRenderWindow();
    void render();
    void handleEvent(const sf::Event& event);
    void pointerDown(io::Pointer::Button button, sf::Vector2f position, int id);
    void pointerDrag(sf::Vector2f position, int id);
    void pointerUp(sf::Vector2f position, int id);
    
    string title;
    int antialiasing;
    bool fullscreen;
    float aspect_ratio; //Aspect ratio in window mode (ignored in full screen)
    sf::Color clear_color;
    
    int mouse_button_down_mask;
    std::map<int, P<GraphicsLayer>> pointer_focus_layer;

    static P<Window> window;
    
    //Engine needs to be able to access the render window, as this is needed for event polling.
    friend class Engine;
    friend class io::Clipboard;
};

};//!namespace sp

#endif//SP2_WINDOW_H
