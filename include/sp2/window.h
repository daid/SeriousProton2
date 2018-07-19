#ifndef SP2_WINDOW_H
#define SP2_WINDOW_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <sp2/math/vector.h>
#include <sp2/pointer.h>
#include <sp2/pointerList.h>
#include <sp2/string.h>
#include <sp2/io/pointer.h>
#include <sp2/graphics/color.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <map>
#include <memory>


namespace sp {
namespace io { class Clipboard; }

class GraphicsLayer;
class Texture;
class MeshData;
class Window : public AutoPointerObject
{
public:
    Window();
    Window(float aspect_ratio);
    
    void setFullScreen(bool fullscreen);
    void setClearColor(Color color);
    void hideCursor();
    void setDefaultCursor();
    void setCursor(Texture* texture, std::shared_ptr<MeshData> mesh);
    
    void addLayer(P<GraphicsLayer> layer);
private:
    sf::RenderWindow render_window;
    RenderQueue queue;

    virtual ~Window();
    
    void createRenderWindow();
    void render();
    void handleEvent(const sf::Event& event);
    void pointerDown(io::Pointer::Button button, Vector2d position, int id);
    void pointerDrag(Vector2d position, int id);
    void pointerUp(Vector2d position, int id);
    Vector2d screenToGLPosition(int x, int y);
    
    PList<GraphicsLayer> graphics_layers;
    
    string title;
    int antialiasing;
    bool fullscreen;
    float aspect_ratio; //Aspect ratio in window mode (ignored in full screen)
    Color clear_color;
    Texture* cursor_texture;
    std::shared_ptr<MeshData> cursor_mesh;
    
    int mouse_button_down_mask;
    std::map<int, P<GraphicsLayer>> pointer_focus_layer;

    static PList<Window> windows;
    static bool anyWindowOpen();
    
    //Engine needs to be able to access the render window, as this is needed for event polling.
    friend class Engine;
    friend class GraphicsLayer;
    friend class io::Clipboard;
};

};//namespace sp

#endif//SP2_WINDOW_H
