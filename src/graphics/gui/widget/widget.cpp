#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/layout/layout.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/audio/sound.h>
#include <sp2/stringutil/convert.h>
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

void Widget::loadThemeStyle(const string& name)
{
    theme_style_name = name;
    theme = Theme::getTheme(theme_name)->getStyle(theme_style_name);
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
    render_data.order = getParent()->render_data.order + 10 + render_order;
    if (render_data_outdated && visible)
    {
        render_data_outdated = false;
        updateRenderData();
    }
}

void Widget::postLoading()
{
}

bool Widget::onPointerMove(Vector2d position, int id)
{
    return false;
}

void Widget::onPointerLeave(int id)
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
    layout.match_content_size = false;
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
    for(P<Widget> w : widget->getChildren())
    {
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

void Widget::setID(const string& id)
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
        layout.match_content_size = false;
    }
    else if (key == "width")
    {
        layout.size.x = stringutil::convert::toFloat(value);
        layout.match_content_size = false;
    }
    else if (key == "height")
    {
        layout.size.y = stringutil::convert::toFloat(value);
        layout.match_content_size = false;
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
        if (layout.match_content_size)
            LOG(Warning, "match_content_size was set for widget:", id, "but default is set unless a size is specified.");
    }
    else if (key == "stretch")
    {
        if (value == "aspect")
            layout.fill_height = layout.fill_width = layout.lock_aspect_ratio = true;
        else
            layout.fill_height = layout.fill_width = stringutil::convert::toBool(value);
        layout.match_content_size = false;
    }
    else if (key == "fill_height")
    {
        layout.fill_height = stringutil::convert::toBool(value);
        layout.match_content_size = false;
    }
    else if (key == "fill_width")
    {
        layout.fill_width = stringutil::convert::toBool(value);
        layout.match_content_size = false;
    }
    else if (key == "theme")
    {
        if (theme_name != value)
        {
            theme_name = value;
            theme = Theme::getTheme(theme_name)->getStyle(theme_style_name);
        }
    }
    else if (key == "theme_data" || key == "style")
    {
        if (theme_style_name != value)
        {
            theme_style_name = value;
            theme = Theme::getTheme(theme_name)->getStyle(theme_style_name);
            markRenderDataOutdated();
        }
    }
    else if (key == "visible")
    {
        setVisible(stringutil::convert::toBool(value));
    }
    else if (key == "enabled")
    {
        setEnable(stringutil::convert::toBool(value));
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
    for(P<Widget> w : getChildren())
    {
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
        layout_manager->updateLoop(this, Rect2d(Vector2d(layout.padding.left, layout.padding.top), size - padding_size));
        if (layout.match_content_size)
        {
            Vector2d content_size_min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            Vector2d content_size_max(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
            for(P<Widget> w : getChildren())
            {
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
    {
        Callback c = callback;
        c(v);
    }
}

void Widget::playThemeSound(State state)
{
    if (!theme->states[int(state)].sound.empty())
        sp::audio::Sound::play(theme->states[int(state)].sound);
}

void Widget::updateRenderDataToThemeImage(int flags)
{
    updateRenderDataToThemeImage(Rect2f(Vector2f(0, 0), Vector2f(getRenderSize())), Rect2f(Vector2f(0, 0), Vector2f(getRenderSize())), flags);
}

void Widget::updateRenderDataToThemeImage(Rect2f rect, int flags)
{
    updateRenderDataToThemeImage(rect, rect, flags);
}

void Widget::updateRenderDataToThemeImage(Rect2f rect, Rect2f clip, int flags)
{
    const ThemeStyle::StateStyle& t = theme->states[int(getState())];

    render_data.texture = t.texture;
    if (render_data.texture)
        render_data.shader = Shader::get("internal:basic.shader");
    else
        render_data.shader = Shader::get("internal:color.shader");
    render_data.color = t.color;

    MeshData::Vertices vertices;
    MeshData::Indices indices{0, 1, 4, 1, 5, 4, 1, 2, 5, 2, 6, 5, 2, 3, 6, 3, 7, 6, 4, 5, 8, 5, 9, 8, 5, 6, 9, 6, 10, 9, 6, 7, 10, 7, 11, 10, 8, 9, 12, 9, 13, 12, 9, 10, 13, 10, 14, 13, 10, 11, 14, 11, 15, 14};
    vertices.reserve(16);

    if (clip.overlaps(rect))
    {
        float corner_size = t.size;
        if (!(flags & render_flag_horizontal_corner_clip))
            corner_size = std::min(corner_size, rect.size.x * 0.5f);
        if (!(flags & render_flag_vertical_corner_clip))
            corner_size = std::min(corner_size, rect.size.y * 0.5f);

        Vector2f v0(rect.position);
        Vector2f v1(v0 + Vector2f(std::min(corner_size, rect.size.x * 0.5f), std::min(corner_size, rect.size.y * 0.5f)));
        Vector2f v3(v0 + Vector2f(rect.size));
        Vector2f v2(v3 - Vector2f(std::min(corner_size, rect.size.x * 0.5f), std::min(corner_size, rect.size.y * 0.5f)));

        v0.x = std::min(std::max(v0.x, float(clip.position.x)), float(clip.position.x + clip.size.x));
        v0.y = std::min(std::max(v0.y, float(clip.position.y)), float(clip.position.y + clip.size.y));
        v1.x = std::min(std::max(v1.x, float(clip.position.x)), float(clip.position.x + clip.size.x));
        v1.y = std::min(std::max(v1.y, float(clip.position.y)), float(clip.position.y + clip.size.y));
        v2.x = std::min(std::max(v2.x, float(clip.position.x)), float(clip.position.x + clip.size.x));
        v2.y = std::min(std::max(v2.y, float(clip.position.y)), float(clip.position.y + clip.size.y));
        v3.x = std::min(std::max(v3.x, float(clip.position.x)), float(clip.position.x + clip.size.x));
        v3.y = std::min(std::max(v3.y, float(clip.position.y)), float(clip.position.y + clip.size.y));

        Vector2f uv0(0.5 * (v0.x - rect.position.x) / corner_size, 1.0 - 0.5 * (v0.y - rect.position.y) / corner_size);
        Vector2f uv1(0.5 * (v1.x - rect.position.x) / corner_size, 1.0 - 0.5 * (v1.y - rect.position.y) / corner_size);
        Vector2f uv2(1.0 - 0.5 * (rect.position.x + rect.size.x - v2.x) / corner_size, 0.5 * (rect.position.y + rect.size.y - v2.y) / corner_size);
        Vector2f uv3(1.0 - 0.5 * (rect.position.x + rect.size.x - v3.x) / corner_size, 0.5 * (rect.position.y + rect.size.y - v3.y) / corner_size);

        vertices.emplace_back(Vector3f(v0.x, v0.y, 0), Vector2f(uv0.x, uv0.y));
        vertices.emplace_back(Vector3f(v1.x, v0.y, 0), Vector2f(uv1.x, uv0.y));
        vertices.emplace_back(Vector3f(v2.x, v0.y, 0), Vector2f(uv2.x, uv0.y));
        vertices.emplace_back(Vector3f(v3.x, v0.y, 0), Vector2f(uv3.x, uv0.y));

        vertices.emplace_back(Vector3f(v0.x, v1.y, 0), Vector2f(uv0.x, uv1.y));
        vertices.emplace_back(Vector3f(v1.x, v1.y, 0), Vector2f(uv1.x, uv1.y));
        vertices.emplace_back(Vector3f(v2.x, v1.y, 0), Vector2f(uv2.x, uv1.y));
        vertices.emplace_back(Vector3f(v3.x, v1.y, 0), Vector2f(uv3.x, uv1.y));

        vertices.emplace_back(Vector3f(v0.x, v2.y, 0), Vector2f(uv0.x, uv2.y));
        vertices.emplace_back(Vector3f(v1.x, v2.y, 0), Vector2f(uv1.x, uv2.y));
        vertices.emplace_back(Vector3f(v2.x, v2.y, 0), Vector2f(uv2.x, uv2.y));
        vertices.emplace_back(Vector3f(v3.x, v2.y, 0), Vector2f(uv3.x, uv2.y));

        vertices.emplace_back(Vector3f(v0.x, v3.y, 0), Vector2f(uv0.x, uv3.y));
        vertices.emplace_back(Vector3f(v1.x, v3.y, 0), Vector2f(uv1.x, uv3.y));
        vertices.emplace_back(Vector3f(v2.x, v3.y, 0), Vector2f(uv2.x, uv3.y));
        vertices.emplace_back(Vector3f(v3.x, v3.y, 0), Vector2f(uv3.x, uv3.y));

        render_data.mesh = MeshData::create(std::move(vertices), std::move(indices));
    }
    else
    {
        render_data.mesh = nullptr;
    }
}

}//namespace gui
}//namespace sp
