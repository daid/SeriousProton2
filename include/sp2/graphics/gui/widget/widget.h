#ifndef SP2_GRAPHICS_GUI_WIDGET_H
#define SP2_GRAPHICS_GUI_WIDGET_H

#include <sp2/graphics/gui/container.h>
#include <sp2/graphics/color.h>
#include <sp2/math/vector.h>
#include <sp2/pointerVector.h>
#include <sp2/string.h>
#include <sp2/variant.h>
#include <sp2/io/pointer.h>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>

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
        sf::Vector2i span;
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
    void setEnable(bool enable);
    void enable();
    void disable();
    bool isEnabled();
    void setID(string id);
    
    void setEventCallback(Callback callback);
    virtual void setAttribute(const string& key, const string& value);
    
    P<Widget> getWidgetWithID(const string& id);
    template<class T> P<T> getWidgetAt(sp::Vector2f position)
    {
        if (layout.rect.contains(position))
        {
            for(PList<Widget>::ReverseIterator it = children.rbegin(); it != children.rend(); ++it)
            {
                Widget* w = *it;
                if (w->isVisible())
                {
                    P<T> result = w->getWidgetAt<T>(position);
                    if (result)
                        return result;
                }
            }
            return P<Widget>(this);
        }
        return nullptr;
    }
    
    LayoutInfo layout;

#ifdef DEBUG
    void setupAutoReload(P<Widget> widget, const string& resource_name, const string& root_id);
#endif
protected:
    const ThemeData* theme;
    
    void loadThemeData(string name);
    void setFocusable(bool value);
protected:
    void runCallback(Variant v);

    void renderStretched(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, sp::Color color);
    void renderStretchedH(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, sp::Color color);
    void renderStretchedV(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, sp::Color color);
    void renderStretchedHV(sf::RenderTarget& window, const sf::FloatRect& rect, float corner_size, const string& texture, sp::Color color);
    void renderText(sf::RenderTarget& window, const sf::FloatRect& rect, Alignment alignment, const string& text, const string& font_name, float text_size, sp::Color color);
    void renderTextVertical(sf::RenderTarget& window, const sf::FloatRect& rect, Alignment alignment, const string& text, const string& font_name, float text_size, sp::Color color);
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

#ifdef DEBUG
    class AutoReloadData
    {
    public:
        sf::Time last_modify_time;
        string resource_name;
        string root_id;
        P<Widget> widget;
    };
    std::vector<AutoReloadData> auto_reload;
#endif

    static float text_scale_factor;

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
