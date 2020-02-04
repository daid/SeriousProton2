#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/stringutil/convert.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("label", Label);

Label::Label(P<Widget> parent, const string& theme_style_name)
: Widget(parent)
{
    loadThemeStyle(theme_style_name);
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
    else if (key == "word_wrap" || key == "line_wrap")
    {
        line_wrap = stringutil::convert::toBool(value);
        markRenderDataOutdated();
    }
    else if (key == "scale_to_text")
    {
        scale_to_text = stringutil::convert::toBool(value);
        markRenderDataOutdated();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Label::updateRenderData()
{
    const ThemeStyle::StateStyle& t = theme->states[int(getState())];

    render_data.shader = Shader::get("internal:basic.shader");
    if (t.font)
    {
        //TODO: Vertical
        sp::Font::PreparedFontString prepared;
        prepared = t.font->prepare(label, 64, text_size < 0 ? t.size : text_size, getRenderSize(), text_alignment, line_wrap ? Font::FlagLineWrap : 0);
        if (scale_to_text)
        {
            if (line_wrap)
                layout.size.y = prepared.getUsedAreaSize().y;
            else
                layout.size = sp::Vector2d(prepared.getUsedAreaSize());
        }
        render_data.mesh = prepared.create();
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
