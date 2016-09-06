#ifndef SP2_GRAPHICS_GUI_WIDGET_H
#define SP2_GRAPHICS_GUI_WIDGET_H

#include <sp2/graphics/gui/container.h>
#include <sp2/math/vector.h>
#include <sp2/pointerVector.h>
#include <sp2/string.h>
#include <SFML/Graphics/RenderTarget.hpp>

namespace sp {
namespace gui {

class ThemeData;
class Widget : public Container
{
public:
    Widget(P<Widget> parent);

    enum class Alignment
    {
        TopLeft,
        Top,
        TopRight,
        Left,
        Center,
        Right,
        BottomLeft,
        Bottom,
        BottomRight
    };
    enum class State
    {
        Normal,
        Disabled,
        Focused,
        Hovered,
        
        Count
    };
    
    virtual void render(sf::RenderTarget& window);
    State getState();
    
    void setPosition(float x, float y, Alignment alignment);
    void setPosition(sf::Vector2f v, Alignment alignment);
    void setSize(float x, float y);
    void setSize(sf::Vector2f v);
protected:
    class LayoutInfo
    {
    public:
        Vector2f position;
        Alignment alignment;
        Vector2f size;
        float margin_left, margin_right, margin_top, margin_bottom;
        Vector2f min_size;
        Vector2f max_size;
        bool fill_width;
        bool fill_height;
        
        P<Widget> anchor_widget;
        Alignment anchor_point;
        
        sf::FloatRect rect;
    } layout;

    const ThemeData* theme;
    
    void loadThemeData(string name);
private:
    Widget();

    P<Widget> parent;
    Layout* layout_manager;
    bool enabled;
    bool focus;
    bool hover;

    string theme_name;
    string theme_data_name;
    
    void updateLayout();

    friend class GraphicsLayer;
    friend class Layout;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_WIDGET_H
