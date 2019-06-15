#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/layout/layout.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/assert.h>
#include <limits>

namespace sp {
namespace gui {

WidgetClassRegistry* WidgetClassRegistry::first;

SP_REGISTER_WIDGET("", Widget);

Widget::Widget(P<Widget> parent)
: Node(parent)
{
    theme_name = parent->theme_name;
    render_data.type = parent->render_data.type;
    render_order = 0;
}

Widget::Widget(P<Node> parent)
: Node(parent)
{
    render_data.type = RenderData::Type::Normal;
}

Widget::~Widget()
{
    if (layout_manager)
        delete layout_manager;
}

void Widget::loadThemeData(string name)
{
    theme_data_name = name;
    theme = Theme::getTheme(theme_name)->getData(theme_data_name);
}

void Widget::setFocusable(bool value)
{
    focusable = value;
    if (!focusable && focus)
    {
        render_data_outdated = true;
        focus = false;
    }
}

void Widget::updateRenderData()
{
}

void Widget::onUpdate(float delta)
{
    if (render_data_outdated && visible)
    {
        render_data_outdated = false;
        render_data.order = getParent()->render_data.order + 10 + render_order;
        updateRenderData();
    }
}

void Widget::postLoading()
{
}

bool Widget::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    return false;
}

void Widget::onPointerDrag(Vector2d position, int id)
{
}

void Widget::onPointerUp(Vector2d position, int id)
{
}

void Widget::onTextInput(const string& text)
{
}

void Widget::onTextInput(TextInputEvent e)
{
}

Widget::State Widget::getState() const
{
    if (!enabled)
        return State::Disabled;
    if (hover)
        return State::Hovered;
    if (focus)
        return State::Focused;
    return State::Normal;
}

void Widget::setPosition(float x, float y, Alignment alignment)
{
    layout.position.x = x;
    layout.position.y = y;
    layout.alignment = alignment;
}

void Widget::setPosition(Vector2d v, Alignment alignment)
{
    layout.position = v;
    layout.alignment = alignment;
}

void Widget::setSize(float x, float y)
{
    layout.size.x = x;
    layout.size.y = y;
}

void Widget::setSize(Vector2d v)
{
    layout.size = v;
}

static void recursiveSetRenderType(P<Widget> widget, RenderData::Type type)
{
    widget->render_data.type = type;
    for(Node* child : widget->getChildren())
    {
        P<Widget> w = P<Node>(child);
        if (!w)
            continue;
        if (w->isVisible())
            recursiveSetRenderType(w, type);
    }
}

void Widget::setVisible(bool visible)
{
    if (this->visible == visible)
        return;
    this->visible = visible;
    
    if (visible)
        recursiveSetRenderType(this, RenderData::Type::Normal);
    else
        recursiveSetRenderType(this, RenderData::Type::None);
}

void Widget::show()
{
    setVisible(true);
}

void Widget::hide()
{
    setVisible(false);
}

bool Widget::isVisible()
{
    return visible;
}

void Widget::setEnable(bool enable)
{
    if (enabled != enable) markRenderDataOutdated();
    enabled = enable;
}

void Widget::enable()
{
    if (!enabled) markRenderDataOutdated();
    enabled = true;
}

void Widget::disable()
{
    if (enabled) markRenderDataOutdated();
    enabled = false;
}

bool Widget::isEnabled()
{
    return enabled;
}

bool Widget::isFocused()
{
    return focus;
}

void Widget::setID(string id)
{
    this->id = id;
}

void Widget::setEventCallback(Callback callback)
{
    this->callback = callback;
}

void Widget::setAttribute(const string& key, const string& value)
{
    if (key == "size")
    {
        layout.size = stringutil::convert::toVector2d(value);
    }
    else if (key == "width")
    {
        layout.size.x = stringutil::convert::toFloat(value);
    }
    else if (key == "height")
    {
        layout.size.y = stringutil::convert::toFloat(value);
    }
    else if (key == "position")
    {
        layout.position = stringutil::convert::toVector2d(value);
    }
    else if (key == "margin")
    {
        auto values = value.split(",", 3);
        if (values.size() == 1)
        {
            layout.margin.top = layout.margin.bottom = layout.margin.left = layout.margin.right = stringutil::convert::toFloat(values[0].strip());
        }
        else if (values.size() == 2)
        {
            layout.margin.left = layout.margin.right = stringutil::convert::toFloat(values[0].strip());
            layout.margin.top = layout.margin.bottom = stringutil::convert::toFloat(values[1].strip());
        }
        else if (values.size() == 3)
        {
            layout.margin.left = layout.margin.right = stringutil::convert::toFloat(values[0].strip());
            layout.margin.top = stringutil::convert::toFloat(values[1].strip());
            layout.margin.bottom = stringutil::convert::toFloat(values[2].strip());
        }
        else if (values.size() == 4)
        {
            layout.margin.left = stringutil::convert::toFloat(values[0].strip());
            layout.margin.right = stringutil::convert::toFloat(values[1].strip());
            layout.margin.top = stringutil::convert::toFloat(values[2].strip());
            layout.margin.bottom = stringutil::convert::toFloat(values[3].strip());
        }
    }
    else if (key == "padding")
    {
        auto values = value.split(",", 3);
        if (values.size() == 1)
        {
            layout.padding.top = layout.padding.bottom = layout.padding.left = layout.padding.right = stringutil::convert::toFloat(values[0].strip());
        }
        else if (values.size() == 2)
        {
            layout.padding.left = layout.padding.right = stringutil::convert::toFloat(values[0].strip());
            layout.padding.top = layout.padding.bottom = stringutil::convert::toFloat(values[1].strip());
        }
        else if (values.size() == 3)
        {
            layout.padding.left = layout.padding.right = stringutil::convert::toFloat(values[0].strip());
            layout.padding.top = stringutil::convert::toFloat(values[1].strip());
            layout.padding.bottom = stringutil::convert::toFloat(values[2].strip());
        }
        else if (values.size() == 4)
        {
            layout.padding.left = stringutil::convert::toFloat(values[0].strip());
            layout.padding.right = stringutil::convert::toFloat(values[1].strip());
            layout.padding.top = stringutil::convert::toFloat(values[2].strip());
            layout.padding.bottom = stringutil::convert::toFloat(values[3].strip());
        }
    }
    else if (key == "span")
    {
        layout.span = stringutil::convert::toVector2i(value);
    }
    else if (key == "alignment")
    {
        layout.alignment = stringutil::convert::toAlignment(value);
    }
    else if (key == "layout")
    {
        LayoutClassRegistry* reg;
        for(reg = LayoutClassRegistry::first; reg != nullptr; reg = reg->next)
        {
            if (value == reg->name)
                break;
        }
        if (reg)
        {
            if (layout_manager)
                delete layout_manager;
            layout_manager = reg->creation_function();
        }else{
            LOG(Error, "Failed to find layout type:", value);
        }
    }
    else if (key == "match_content_size")
    {
        layout.match_content_size = stringutil::convert::toBool(value);
    }
    else if (key == "stretch")
    {
        layout.fill_height = layout.fill_width = stringutil::convert::toBool(value);
    }
    else if (key == "fill_height")
    {
        layout.fill_height = stringutil::convert::toBool(value);
    }
    else if (key == "fill_width")
    {
        layout.fill_width = stringutil::convert::toBool(value);
    }
    else if (key == "theme")
    {
        if (theme_name != value)
        {
            theme_name = value;
            theme = Theme::getTheme(theme_name)->getData(theme_data_name);
        }
    }
    else if (key == "theme_data")
    {
        if (theme_data_name != value)
        {
            theme_data_name = value;
            theme = Theme::getTheme(theme_name)->getData(theme_data_name);
            markRenderDataOutdated();
        }
    }
    else if (key == "visible")
    {
        setVisible(stringutil::convert::toBool(value));
    }
    else if (key == "order")
    {
        render_order = stringutil::convert::toInt(value);
    }
    else if (key == "tag")
    {
        tag = value;
    }
    else
    {
        if (id != "")
            LOG(Warning, "Tried to set unknown widget attribute:", key, "to", value, "on", id);
        else
            LOG(Warning, "Tried to set unknown widget attribute:", key, "to", value);
    }
}

P<Widget> Widget::getWidgetWithID(const string& id)
{
    if (this->id == id)
        return this;
    for(Node* child : getChildren())
    {
        P<Widget> w = P<Node>(child);
        if (!w)
            continue;
        w = w->getWidgetWithID(id);
        if (w)
            return w;
    }
    return nullptr;
}

#ifdef DEBUG
void Widget::setupAutoReload(P<Widget> widget, const string& resource_name, const string& root_id)
{
    AutoReloadData data;
    data.last_modify_time = io::ResourceProvider::getModifyTime(resource_name);
    data.resource_name = resource_name;
    data.root_id = root_id;
    data.widget = widget;
    auto_reload.push_back(data);
}
#endif

void Widget::updateLayout(Vector2d position, Vector2d size)
{
#ifdef DEBUG
    for(auto& data : auto_reload)
    {
        std::chrono::system_clock::time_point modify_time = io::ResourceProvider::getModifyTime(data.resource_name);
        if (modify_time == data.last_modify_time)
            continue;
        LOG(Info, "Reloading:", data.resource_name, data.root_id);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        data.widget.destroy();
        data.widget = sp::gui::Loader::load(data.resource_name, data.root_id, this);
        if (data.widget)
            data.last_modify_time = modify_time;
    }
#endif
    if (layout_manager || !getChildren().empty())
    {
        if (!layout_manager)
            layout_manager = new Layout();

        Vector2d padding_size(layout.padding.left + layout.padding.right, layout.padding.top + layout.padding.bottom);
        layout_manager->update(this, Rect2d(Vector2d(layout.padding.left, layout.padding.top), size - padding_size));
        if (layout.match_content_size)
        {
            Vector2d content_size_min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            Vector2d content_size_max(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
            for(Node* child : getChildren())
            {
                P<Widget> w = P<Node>(child);
                if (w && w->isVisible())
                {
                    Vector2d p0 = w->getPosition2D();
                    Vector2d p1 = p0 + w->getRenderSize();
                    content_size_min.x = std::min(content_size_min.x, p0.x - w->layout.margin.left);
                    content_size_min.y = std::min(content_size_min.y, p0.y - w->layout.margin.bottom);
                    content_size_max.x = std::max(content_size_max.x, p1.x + w->layout.margin.right);
                    content_size_max.y = std::max(content_size_max.y, p1.y + w->layout.margin.top);
                }
            }
            if (content_size_max.x != std::numeric_limits<float>::min())
            {
                size = (content_size_max - content_size_min) + padding_size;
                layout.size = size;
            }
        }
    }
    if (getPosition2D() != position || getRenderSize() != size)
    {
        Node::setPosition(position);
        render_size = size;
        render_data_outdated = true;
    }
}

void Widget::runCallback(Variant v)
{
    if (callback)
        callback(v);
}

std::shared_ptr<MeshData> Widget::createStretched(Vector2d size)
{
    if (size.x >= size.y)
    {
        return createStretchedH(size);
    }else{
        return createStretchedV(size);
    }
}

std::shared_ptr<MeshData> Widget::createStretchedH(Vector2d size)
{
    MeshData::Vertices vertices;
    MeshData::Indices indices{0,2,1, 1,2,3, 2,4,3, 3,4,5, 4,6,5, 5,6,7};
    vertices.reserve(8);
    
    float w = size.y / 2.0;
    float u = 0.5;
    if (w > size.x / 2.0)
    {
        u = 0.5 * size.x / size.y;
        w = size.x / 2.0;
    }
    vertices.emplace_back(Vector3f(0, 0, 0), Vector2f(0, 1));
    vertices.emplace_back(Vector3f(0, size.y, 0), Vector2f(0, 0));
    vertices.emplace_back(Vector3f(w, 0, 0), Vector2f(u, 1));
    vertices.emplace_back(Vector3f(w, size.y, 0), Vector2f(u, 0));
    vertices.emplace_back(Vector3f(size.x-w, 0, 0), Vector2f(1.0 - u, 1));
    vertices.emplace_back(Vector3f(size.x-w, size.y, 0), Vector2f(1.0 - u, 0));
    vertices.emplace_back(Vector3f(size.x, 0, 0), Vector2f(1, 1));
    vertices.emplace_back(Vector3f(size.x, size.y, 0), Vector2f(1, 0));
    
    return MeshData::create(std::move(vertices), std::move(indices));
}

std::shared_ptr<MeshData> Widget::createStretchedV(Vector2d size)
{
    MeshData::Vertices vertices;
    MeshData::Indices indices{0,1,2, 1,3,2, 2,3,4, 3,5,4, 4,5,6, 5,7,6};
    vertices.reserve(8);
    
    float h = size.x / 2.0;
    float v = 0.5;
    if (h > size.y / 2.0)
    {
        v = 0.5 * size.y / size.x;
        h = size.y / 2.0;
    }
    vertices.emplace_back(Vector3f(0, 0, 0), Vector2f(0, 1));
    vertices.emplace_back(Vector3f(size.x, 0, 0), Vector2f(1, 1));
    vertices.emplace_back(Vector3f(0, h, 0), Vector2f(0, 1.0-v));
    vertices.emplace_back(Vector3f(size.x, h, 0), Vector2f(1, 1.0-v));
    vertices.emplace_back(Vector3f(0, size.y-h, 0), Vector2f(0, v));
    vertices.emplace_back(Vector3f(size.x, size.y-h, 0), Vector2f(1, v));
    vertices.emplace_back(Vector3f(0, size.y, 0), Vector2f(0, 0));
    vertices.emplace_back(Vector3f(size.x, size.y, 0), Vector2f(1, 0));
    
    return MeshData::create(std::move(vertices), std::move(indices));
}

std::shared_ptr<MeshData> Widget::createStretchedHV(Vector2d size, double corner_size)
{
    MeshData::Vertices vertices;
    MeshData::Indices indices{0, 1, 4, 1, 5, 4, 1, 2, 5, 2, 6, 5, 2, 3, 6, 3, 7, 6, 4, 5, 8, 5, 9, 8, 5, 6, 9, 6, 10, 9, 6, 7, 10, 7, 11, 10, 8, 9, 12, 9, 13, 12, 9, 10, 13, 10, 14, 13, 10, 11, 14, 11, 15, 14};
    vertices.reserve(16);

    corner_size = std::min(corner_size, size.x / 2.0f);
    corner_size = std::min(corner_size, size.y / 2.0f);

    vertices.emplace_back(Vector3f(0, 0, 0), Vector2f(0, 1));
    vertices.emplace_back(Vector3f(corner_size, 0, 0), Vector2f(0.5, 1));
    vertices.emplace_back(Vector3f(size.x - corner_size, 0, 0), Vector2f(0.5, 1));
    vertices.emplace_back(Vector3f(size.x, 0, 0), Vector2f(1, 1));

    vertices.emplace_back(Vector3f(0, corner_size, 0), Vector2f(0, 0.5));
    vertices.emplace_back(Vector3f(corner_size, corner_size, 0), Vector2f(0.5, 0.5));
    vertices.emplace_back(Vector3f(size.x - corner_size, corner_size, 0), Vector2f(0.5, 0.5));
    vertices.emplace_back(Vector3f(size.x, corner_size, 0), Vector2f(1, 0.5));

    vertices.emplace_back(Vector3f(0, size.y - corner_size, 0), Vector2f(0, 0.5));
    vertices.emplace_back(Vector3f(corner_size, size.y - corner_size, 0), Vector2f(0.5, 0.5));
    vertices.emplace_back(Vector3f(size.x - corner_size, size.y - corner_size, 0), Vector2f(0.5, 0.5));
    vertices.emplace_back(Vector3f(size.x, size.y - corner_size, 0), Vector2f(1, 0.5));

    vertices.emplace_back(Vector3f(0, size.y, 0), Vector2f(0, 0));
    vertices.emplace_back(Vector3f(corner_size, size.y, 0), Vector2f(0.5, 0));
    vertices.emplace_back(Vector3f(size.x - corner_size, size.y, 0), Vector2f(0.5, 0));
    vertices.emplace_back(Vector3f(size.x, size.y, 0), Vector2f(1, 0));

    return MeshData::create(std::move(vertices), std::move(indices));
}

};//namespace gui
};//namespace sp
