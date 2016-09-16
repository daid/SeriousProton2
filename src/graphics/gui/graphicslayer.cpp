#include <sp2/graphics/gui/graphicslayer.h>
#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/logging.h>
#include <limits>

#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

P<GraphicsLayer> GraphicsLayer::default_gui_layer;

GraphicsLayer::GraphicsLayer(int priority)
: sp::GraphicsLayer(priority)
{
    root = new Widget();

    min_size.x = min_size.y = 0.0;
    max_size.x = max_size.y = std::numeric_limits<float>::max();
    
    if (!default_gui_layer)
        default_gui_layer = this;
}

void GraphicsLayer::setFixedVirtualSize(sf::Vector2f size)
{
    min_size = size;
    max_size = size;
}

void GraphicsLayer::setMinimalVirtualSize(sf::Vector2f size)
{
    min_size = size;
}

void GraphicsLayer::setMaximumVirtualSize(sf::Vector2f size)
{
    max_size = size;
}

void GraphicsLayer::render(sf::RenderTarget& window)
{
    sf::Vector2f window_size = sf::Vector2f(window.getSize());
    sf::Vector2f virtual_size = window_size;
    //Setup a virtual size which matches the min/max specs that we have. Getting as close to the actual window size a possible.
    virtual_size.x = std::max(min_size.x, virtual_size.x);
    virtual_size.y = std::max(min_size.y, virtual_size.y);
    virtual_size.x = std::min(max_size.x, virtual_size.x);
    virtual_size.y = std::min(max_size.y, virtual_size.y);
        
    sf::View view(sf::FloatRect(0, 0, virtual_size.x, virtual_size.y));
    //Now we need to setup the viewport so our virtual window is on the screen, and in the center.
    //Maintaining the same aspect ratio as the virtual size.
    float aspect_x = window_size.x / virtual_size.x;
    float aspect_y = window_size.y / virtual_size.y;
    if (aspect_y < aspect_x)
    {
        view.setViewport(sf::FloatRect(0.5f - 0.5f * (aspect_y / aspect_x), 0, aspect_y / aspect_x, 1));
    }else{
        view.setViewport(sf::FloatRect(0, 0.5f - 0.5f * (aspect_x / aspect_y), 1, aspect_x / aspect_y));
    }
    window.setView(view);
    
    virtual_position_transform = sf::Transform::Identity;
    virtual_position_transform.scale(virtual_size.x, virtual_size.y);
    virtual_position_transform.scale(1.0f / view.getViewport().width, 1.0f / view.getViewport().height);
    virtual_position_transform.translate(-view.getViewport().left, -view.getViewport().top);
    
    root->layout.position = sf::Vector2f(0, 0);
    root->layout.size = virtual_size;
    root->layout.rect = sf::FloatRect(0, 0, virtual_size.x, virtual_size.y);
    root->updateLayout();
    
    drawWidgets(window, root);
}

bool GraphicsLayer::onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id)
{
    position = virtual_position_transform.transformPoint(position);
    P<Widget> w = widgetAtPosition(root, position);
    while(w)
    {
        if (w->onPointerDown(button, position, id))
        {
            if (focus_widget)
                focus_widget->focus = false;
            focus_widget = w;
            focus_widget->focus = true;
            pointer_widget[id] = w;
            return true;
        }
        w = w->parent;
    }
    if (focus_widget)
    {
        focus_widget->focus = false;
        focus_widget = nullptr;
    }
    return false;
}

void GraphicsLayer::onPointerDrag(sf::Vector2f position, int id)
{
    position = virtual_position_transform.transformPoint(position);
    auto it = pointer_widget.find(id);
    if (it != pointer_widget.end() && it->second)
        it->second->onPointerDrag(position, id);
}

void GraphicsLayer::onPointerUp(sf::Vector2f position, int id)
{
    position = virtual_position_transform.transformPoint(position);
    auto it = pointer_widget.find(id);
    if (it != pointer_widget.end() && it->second)
    {
        it->second->onPointerUp(position, id);
        pointer_widget.erase(it);
    }
}

void GraphicsLayer::drawWidgets(sf::RenderTarget& window, P<Widget> w)
{
    w->render(window);
    
    for(Widget* child : w->children)
    {
        drawWidgets(window, child);
    }
}

P<Widget> GraphicsLayer::widgetAtPosition(P<Widget> w, sf::Vector2f position)
{
    if (w->layout.rect.contains(position))
    {
        for(P<Widget> c : w->children)
        {
            P<Widget> result = widgetAtPosition(c, position);
            if (result)
                return result;
        }
        return w;
    }
    return nullptr;
}

};//!namespace gui
};//!namespace sp
