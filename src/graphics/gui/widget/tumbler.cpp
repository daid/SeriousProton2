#include <sp2/graphics/gui/widget/tumbler.h>
#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/io/keybinding.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

SP_REGISTER_WIDGET("tumbler", Tumbler);

Tumbler::Tumbler(P<Widget> parent)
: Widget(parent)
{
    loadThemeStyle("tumbler.background");

    for(int n=0; n<4; n++)
    {
        sp::P<sp::gui::Label> text = new sp::gui::Label(this, "tumbler.forground");
        text->setAttribute("clip", "true");
        texts.add(text);
    }
}

void Tumbler::setAttribute(const string& key, const string& value)
{
    if (key == "text_size" || key == "text.size")
    {
        for(auto text : texts)
            text->setAttribute(key, value);
    }
    else if (key == "style" || key == "theme_data")
    {
        Widget::setAttribute("style", value + ".background");
        for(auto text : texts)
            text->setAttribute("style", value + ".forground");
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
    const ThemeStyle::StateStyle& t = theme->states[int(getState())];

    render_data.shader = Shader::get("internal:basic.shader");
    render_data.texture = t.texture;
    if (t.texture)
        render_data.mesh = createStretched(getRenderSize());
    else
        render_data.mesh = nullptr;
    render_data.color = t.color;

    Rect2d r(0, -getRenderSize().y * 0.5 - scroll_offset, getRenderSize().x, getRenderSize().y);
    for(auto text : texts)
    {
        Rect2d tmp(r);
        tmp.shrinkToFitWithin(Rect2d(0, 0, getRenderSize().x, getRenderSize().y));
        text->layout.position = tmp.position;
        text->layout.size = tmp.size;
        r.position.y += r.size.y * 0.5;
    }

    if (items.size() > 0)
    {
        int n=active_index;
        for(auto text : texts)
            text->setLabel(items[(n++) % items.size()]);
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
    if (scroll_offset < getRenderSize().y * 0.25)
        scroll_offset = 0.0;
    else
        scroll_offset = getRenderSize().y * 0.5;
    updateOffset();

    if (position.x >= 0 && position.x <= getRenderSize().x && position.y >= 0 && position.y <= getRenderSize().y && isEnabled())
    {
        playThemeSound(State::Hovered);
        if (items.size() > 0)
        {
            runCallback(int((active_index + 1) % items.size()));
            //runCallback(items[(active_index + 1) % items.size()]);
        }
    }
}

void Tumbler::clearItems()
{
    items.clear();
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
        scroll_offset += getRenderSize().y * 0.5;
        active_index -= 1;
    }
    while (scroll_offset > getRenderSize().y * 0.5)
    {
        scroll_offset -= getRenderSize().y * 0.5;
        active_index += 1;
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
