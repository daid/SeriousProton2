#ifndef SP2_GRAPHICS_GUI_WIDGET_H
#define SP2_GRAPHICS_GUI_WIDGET_H

#include <sp2/scene/node.h>
#include <sp2/graphics/color.h>
#include <sp2/math/vector.h>
#include <sp2/string.h>
#include <sp2/variant.h>
#include <sp2/alignment.h>
#include <sp2/io/pointer.h>
#include <sp2/io/textinput.h>


namespace sp {
namespace gui {

class Layout;
class ThemeData;
class Widget : public Node
{
public:
    Widget(P<Widget> parent);
    virtual ~Widget();

    class LayoutInfo
    {
    public:
        class Sides
        {
        public:
            float left = 0;
            float right = 0;
            float top = 0;
            float bottom = 0;
        };
        
        Vector2d position;
        Alignment alignment = Alignment::TopLeft;
        Vector2d size{1, 1};
        Vector2i span{1, 1};
        Sides margin;
        Sides padding;
        Vector2d min_size;
        Vector2d max_size{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        bool fill_width = false;
        bool fill_height = false;
        bool lock_aspect_ratio = false;
        bool match_content_size = false;
        
        P<Widget> anchor_widget;
        Alignment anchor_point = Alignment::TopLeft;
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
    
    virtual void updateRenderData();
    virtual void onUpdate(float delta) override;
    virtual void postLoading();    //Called after the loader finished loading this widget and children have been added.
    virtual bool onPointerDown(io::Pointer::Button button, sp::Vector2d position, int id);
    virtual void onPointerDrag(sp::Vector2d position, int id);
    virtual void onPointerUp(sp::Vector2d position, int id);
    virtual bool enableTextInputOnFocus() const { return false; }
    virtual void onTextInput(const string& text);
    virtual void onTextInput(TextInputEvent e);
    State getState() const;
    
    void setPosition(float x, float y, Alignment alignment);
    void setPosition(sp::Vector2d v, Alignment alignment);
    void setSize(float x, float y);
    void setSize(sp::Vector2d v);
    void setVisible(bool visible);
    void show();
    void hide();
    bool isVisible();
    void setEnable(bool enable);
    void enable();
    void disable();
    bool isEnabled();
    bool isFocused();
    void setID(const string& id);
    const string& getID() { return id; }

    sp::Vector2d getRenderSize() { return render_size; }
    
    void setEventCallback(Callback callback);
    virtual void setAttribute(const string& key, const string& value);
    
    P<Widget> getWidgetWithID(const string& id);

    template<class T, class = typename std::enable_if<std::is_base_of<Widget, T>::value>::type> P<T> getWidgetAt(sp::Vector2d position)
    {
        position -= getPosition2D();
        if (position.x >= 0 && position.x <= render_size.x && position.y >= 0 && position.y <= render_size.y)
        {
            for(PList<Node>::ReverseIterator it = getChildren().rbegin(); it != getChildren().rend(); ++it)
            {
                P<Widget> w = P<Node>(*it);
                if (w && w->isVisible())
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
    string tag; //User set tag, can be used for anything.

#ifdef DEBUG
    void setupAutoReload(P<Widget> widget, const string& resource_name, const string& root_id);
#endif
protected:
    const ThemeData* theme;
    sp::P<Widget> slave_widget;
    
    void loadThemeData(const string& name);
    void setFocusable(bool value);

    void runCallback(Variant v);
    void playThemeSound(State state);
    void markRenderDataOutdated()
    {
        if (slave_widget)
        {
            slave_widget->enabled = enabled;
            slave_widget->focus = focus;
            slave_widget->hover = hover;
            slave_widget->markRenderDataOutdated();
        }
        render_data_outdated = true;
    }

    std::shared_ptr<MeshData> createStretched(Vector2d size);
    std::shared_ptr<MeshData> createStretchedH(Vector2d size);
    std::shared_ptr<MeshData> createStretchedV(Vector2d size);
    std::shared_ptr<MeshData> createStretchedHV(Vector2d size, double corner_size);
private:
    Widget(P<Node> parent);

    Layout* layout_manager = nullptr;
    sp::Vector2d render_size;
    bool visible = true;
    bool enabled = true;
    bool focus = false;
    bool focusable = false;
    bool hover = false;
    bool render_data_outdated = true;
    int render_order;

    string id;
    string theme_name = "default";
    string theme_data_name;
    
    Callback callback;
    
    void updateLayout(Vector2d position, Vector2d size);

#ifdef DEBUG
    class AutoReloadData
    {
    public:
        std::chrono::system_clock::time_point last_modify_time;
        string resource_name;
        string root_id;
        P<Widget> widget;
    };
    std::vector<AutoReloadData> auto_reload;
#endif

    friend class GraphicsLayer;
    friend class Scene;
    friend class RootWidget;
    friend class Layout;
};

class WidgetClassRegistry : NonCopyable 
{
public:
    static WidgetClassRegistry* first;
    
    WidgetClassRegistry* next;
    string name;
    std::function<P<Widget>(P<Widget>)> creation_function;
    
    WidgetClassRegistry(const string& name, std::function<P<Widget>(P<Widget> parent)> creation_function)
    : name(name), creation_function(creation_function)
    {
        next = first;
        first = this;
    }
};

#define SP_REGISTER_WIDGET(name, class_name) \
    sp::gui::WidgetClassRegistry widget_class_registry ## class_name (name, [](sp::P<sp::gui::Widget> parent) { return new class_name(parent); });

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_WIDGET_H
