#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("label", Label);

Label::Label(P<Widget> parent, const string& theme_data_name)
: Widget(parent)
{
    loadThemeData(theme_data_name);
    text_alignment = Alignment::Center;
    vertical = false;
    text_size = -1;
    texture_revision = -1;
    
    setAttribute("order", "-1");
}

void Label::setLabel(const string& label)
{
    if (this->label != label)
    {
        this->label = label;
        markRenderDataOutdated();
    }
}

const string& Label::getLabel()
{
    return label;
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
    else if (key == "text_alignment" || key == "text.alignment")
    {
        text_alignment = stringutil::convert::toAlignment(value);
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
        if (vertical)
            render_data.mesh = t.font->createString(label, 64, text_size < 0 ? t.size : text_size, getRenderSize(), text_alignment);
        else
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

}//namespace gui
}//namespace sp
