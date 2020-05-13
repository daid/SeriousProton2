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
: ItemList(parent)
{
    loadThemeStyle("tumbler.background");
    text_theme = Theme::getTheme("default")->getStyle("tumbler.forground");

    for(int n=0; n<row_count+1; n++)
    {
        text_nodes.add(new sp::Node(this));
    }
}

void Tumbler::setAttribute(const string& key, const string& value)
{
    if (key == "text_size" || key == "text.size")
    {
        text_size = stringutil::convert::toFloat(value);
    }
    else if (key == "style" || key == "theme_data")
    {
        Widget::setAttribute("style", value + ".background");
        text_theme = Theme::getTheme("default")->getStyle(value + ".forground");
    }
    else if (key == "rows")
    {
        row_count = std::max(2, stringutil::convert::toInt(value));
        while(text_nodes.size() < row_count + 1)
            text_nodes.add(new sp::Node(this));
        while(text_nodes.size() > row_count + 1)
            (*text_nodes.begin()).destroy();
    }
    else
    {
        ItemList::setAttribute(key, value);
    }
}

void Tumbler::onUpdate(float delta)
{
    Widget::onUpdate(delta);
    for(auto node : text_nodes)
        node->render_data.type = render_data.type;
}

void Tumbler::updateRenderData()
{
    const ThemeStyle::StateStyle& ft = text_theme->states[int(getState())];

    updateRenderDataToThemeImage();

    if (ft.font)
    {
        int n=active_index - ((row_count - 1) / 2 + 1) + items.size();
        double row_height = getRenderSize().y / double(row_count - 1);
        double offset = scroll_offset - getRenderSize().y * 0.5 - row_height;
        for(auto node : text_nodes)
        {
            node->render_data.shader = render_data.shader;
            node->render_data.order = render_data.order + 1;
            if (items.size() > 0)
            {
                auto text = ft.font->prepare(items[(n++) % items.size()].label, 32, text_size > 0.0 ? text_size : ft.size, getRenderSize(), Alignment::Center, Font::FlagClip);
                for(auto& d : text.data)
                    d.position.y += offset;
                node->render_data.mesh = text.create();
                node->render_data.color = ft.color;
                node->render_data.color.a = 1.0 - (std::abs(offset) / getRenderSize().y);
            }
            else
            {
                node->render_data.mesh = nullptr;
            }
            node->render_data.texture = ft.font->getTexture(32);
            offset += row_height;
        }
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
    double row_height = getRenderSize().y / double(row_count - 1);
    if (scroll_offset < row_height * 0.5)
        scroll_offset = 0.0;
    else
        scroll_offset = row_height;
    updateOffset();

    playThemeSound(State::Hovered);
    if (items.size() > 0)
    {
        runCallback(active_index);
    }
}

void Tumbler::updateOffset()
{
    double row_height = getRenderSize().y / double(row_count - 1);
    while (scroll_offset < 0.0)
    {
        scroll_offset += row_height;
        active_index += 1;
    }
    while (scroll_offset >= row_height)
    {
        scroll_offset -= row_height;
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
