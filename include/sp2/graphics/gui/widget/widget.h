#ifndef SP2_GRAPHICS_GUI_WIDGET_H
#define SP2_GRAPHICS_GUI_WIDGET_H

#include <sp2/graphics/gui/container.h>
#include <sp2/math/vector.h>
#include <sp2/pointerVector.h>
#include <sp2/string.h>
#include <sp2/variant.h>
#include <sp2/io/pointer.h>
#include <SFML/Graphics/RenderTarget.hpp>

namespace sp {
namespace gui {

class ThemeData;
class Widget : public Container
{
public:
    Widget(P<Widget> parent);
    virtual ~Widget();

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
        bool lock_aspect_ratio;
        bool match_content_size;
        
        P<Widget> anchor_widget;
        Alignment anchor_point;
        
        sf::FloatRect rect;
    };
    enum class State
    {
        Normal,
        Disabled,
        Focused,
        Hovered,
        
        Count
    };
    typedef std::function<void(Variant value)> Callback;
    
    virtual void render(sf::RenderTarget& window);
    virtual bool onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id);
    virtual void onPointerDrag(sf::Vector2f position, int id);
    virtual void onPointerUp(sf::Vector2f position, int id);
    State getState() const;
    
    void setPosition(float x, float y, Alignment alignment);
    void setPosition(sf::Vector2f v, Alignment alignment);
    void setSize(float x, float y);
    void setSize(sf::Vector2f v);
    void setVisible(bool visible);
    void show();
    void hide();
    bool isVisible();
    void setID(string id);
    
    void setEventCallback(Callback callback);
    virtual void setAttribute(const string& key, const string& value);
    
    P<Widget> getWidgetWithID(const string& id);
    
    LayoutInfo layout;
protected:
    const ThemeData* theme;
    
    void loadThemeData(string name);
    void setFocusable(bool value);
protected:
    void runCallback(Variant v);

    void renderStretched(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, sf::Color color);
    void renderStretchedH(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, sf::Color color);
    void renderStretchedV(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, sf::Color color);
    void renderStretchedHV(sf::RenderTarget& window, const sf::FloatRect& rect, float corner_size, const string& texture, sf::Color color);
    void renderText(sf::RenderTarget& window, const sf::FloatRect& rect, Alignment alignment, const string& text, const string& font_name, float text_size, sf::Color color);
private:
    Widget();

    P<Widget> parent;
    Layout* layout_manager;
    bool visible;
    bool enabled;
    bool focus;
    bool focusable;
    bool hover;

    string id;
    string theme_name;
    string theme_data_name;
    
    Callback callback;
    
    void updateLayout();

    friend class GraphicsLayer;
};

class WidgetClassRegistry : sf::NonCopyable
{
public:
    static WidgetClassRegistry* first;
    
    WidgetClassRegistry* next;
    string name;
    std::function<P<Widget>(P<Widget>)> creation_function;
    
    WidgetClassRegistry(string name, std::function<P<Widget>(P<Widget> parent)> creation_function)
    : name(name), creation_function(creation_function)
    {
        next = first;
        first = this;
    }
};

#define SP_REGISTER_WIDGET(name, class_name) \
    sp::gui::WidgetClassRegistry widget_class_registry ## class_name (name, [](sp::P<sp::gui::Widget> parent) { return new class_name(parent); });

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_WIDGET_H
