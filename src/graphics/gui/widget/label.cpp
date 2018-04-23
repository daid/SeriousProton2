#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("label", Label);

Label::Label(P<Widget> parent, string theme_data_name)
: Widget(parent)
{
    loadThemeData(theme_data_name);
    text_alignment = Alignment::Center;
    vertical = false;
    text_size = -1;
    texture_revision = -1;
    
    setAttribute("order", "-1");
}

void Label::setLabel(string label)
{
    if (this->label != label)
    {
        this->label = label;
        markRenderDataOutdated();
    }
}

void Label::setAttribute(const string& key, const string& value)
{
    if (key == "label" || key == "caption")
    {
        setLabel(value);
    }
    else if (key == "text_size" || key == "text.size")
    {
        text_size = stringutil::convert::toFloat(value);
        markRenderDataOutdated();
    }
    else if (key == "text_alignment")
    {
        if (value == "topleft") text_alignment = Alignment::TopLeft;
        else if (value == "top") text_alignment = Alignment::Top;
        else if (value == "right") text_alignment = Alignment::TopRight;
        else if (value == "left") text_alignment = Alignment::Left;
        else if (value == "center") text_alignment = Alignment::Center;
        else if (value == "right") text_alignment = Alignment::Right;
        else if (value == "bottomleft") text_alignment = Alignment::BottomLeft;
        else if (value == "bottom") text_alignment = Alignment::Bottom;
        else if (value == "bottomright") text_alignment = Alignment::BottomRight;
        markRenderDataOutdated();
    }
    else if (key == "vertical")
    {
        vertical = stringutil::convert::toBool(value);
        markRenderDataOutdated();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Label::updateRenderData()
{
    const ThemeData::StateData& t = theme->states[int(getState())];

    render_data.shader = Shader::get("internal:basic.shader");
    if (t.font)
    {
        render_data.mesh = t.font->createString(label, 64, text_size < 0 ? t.size : text_size, getRenderSize(), text_alignment);
        render_data.texture = t.font->getTexture(64);
        texture_revision = render_data.texture->getRevision();
    }
    render_data.color = t.color;
}

void Label::onUpdate(float delta)
{
    if (render_data.texture && render_data.texture->getRevision() != texture_revision)
        markRenderDataOutdated();
    Widget::onUpdate(delta);
}

};//namespace gui
};//namespace sp
