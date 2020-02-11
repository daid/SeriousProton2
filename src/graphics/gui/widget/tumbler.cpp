#include <sp2/graphics/gui/widget/tumbler.h>
#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/stringutil/convert.h>
#include <sp2/io/keybinding.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

SP_REGISTER_WIDGET("tumbler", Tumbler);

Tumbler::Tumbler(P<Widget> parent)
: Widget(parent)
{
    loadThemeStyle("tumbler.background");
    text_theme = Theme::getTheme("default")->getStyle("tumbler.forground");

    for(int n=0; n<6; n++)
    {
        text_nodes.add(new sp::Node(this));
    }
}

void Tumbler::setAttribute(const string& key, const string& value)
{
    if (key == "text_size" || key == "text.size")
    {
        text_size = sp::stringutil::convert::toFloat(value);
    }
    else if (key == "style" || key == "theme_data")
    {
        Widget::setAttribute("style", value + ".background");
        text_theme = Theme::getTheme("default")->getStyle(value + ".forground");
    }
    else if (key == "items")
    {
        items = value.split(",");
        for(auto& e : items)
            e = e.strip();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Tumbler::updateRenderData()
{
    const ThemeStyle::StateStyle& bt = theme->states[int(getState())];
    const ThemeStyle::StateStyle& ft = text_theme->states[int(getState())];

    render_data.shader = Shader::get("internal:basic.shader");
    render_data.texture = bt.texture;
    if (bt.texture)
        render_data.mesh = createStretched(getRenderSize());
    else
        render_data.mesh = nullptr;
    render_data.color = bt.color;

    int n=active_index - 3 + items.size();
    double offset = scroll_offset - getRenderSize().y * 0.75;
    for(auto node : text_nodes)
    {
        node->render_data.type = render_data.type;
        node->render_data.shader = render_data.shader;
        node->render_data.order = render_data.order + 1;
        if (items.size() > 0)
        {
            auto text = ft.font->prepare(items[(n++) % items.size()], 32, text_size > 0.0 ? text_size : ft.size, getRenderSize(), Alignment::Center, Font::FlagClip);
            for(auto& d : text.data)
                d.position.y += offset;
            node->render_data.mesh = text.create();
            node->render_data.color.a = 1.0 - (std::abs(offset) / getRenderSize().y);
        }
        else
        {
            node->render_data.mesh = nullptr;
        }
        node->render_data.texture = ft.font->getTexture(32);
        offset += getRenderSize().y * 0.25;
    }
}

bool Tumbler::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    if (isEnabled())
        playThemeSound(State::Normal);
    pointer_position[id] = position;
    return true;
}

void Tumbler::onPointerDrag(Vector2d position, int id)
{
    scroll_offset += position.y - pointer_position[id].y;
    updateOffset();
    pointer_position[id] = position;
    markRenderDataOutdated();
}

void Tumbler::onPointerUp(Vector2d position, int id)
{
    if (scroll_offset < getRenderSize().y * 0.125)
        scroll_offset = 0.0;
    else
        scroll_offset = getRenderSize().y * 0.25;
    updateOffset();

    playThemeSound(State::Hovered);
    if (items.size() > 0)
    {
        runCallback(active_index);
    }
}

void Tumbler::clearItems()
{
    items.clear();
    active_index = 0;
    markRenderDataOutdated();
}

int Tumbler::addItem(const string& label)
{
    items.push_back(label);
    markRenderDataOutdated();
    return items.size() - 1;
}

void Tumbler::updateOffset()
{
    while (scroll_offset < 0.0)
    {
        scroll_offset += getRenderSize().y * 0.25;
        active_index += 1;
    }
    while (scroll_offset >= getRenderSize().y * 0.25)
    {
        scroll_offset -= getRenderSize().y * 0.25;
        active_index -= 1;
    }
    if (items.size() > 0)
    {
        while(active_index < 0)
            active_index += items.size();
        while(active_index >= int(items.size()))
            active_index -= items.size();
    }
}

}//namespace gui
}//namespace sp
