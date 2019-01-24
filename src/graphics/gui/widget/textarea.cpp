#include <sp2/graphics/gui/widget/textarea.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("textarea", TextArea);

TextArea::TextArea(P<Widget> parent)
: Widget(parent)
{
    loadThemeData("textarea");
    text_size = -1;
    texture_revision = -1;
    setFocusable(true);
    
    setAttribute("order", "-1");
}

void TextArea::setAttribute(const string& key, const string& value)
{
    if (key == "text")
    {
        this->value = value;
    }
    else if (key == "text_size" || key == "text.size")
    {
        text_size = stringutil::convert::toFloat(value);
        markRenderDataOutdated();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void TextArea::updateRenderData()
{
    const ThemeData::StateData& t = theme->states[int(getState())];

    render_data.shader = Shader::get("internal:basic.shader");
    if (t.font)
    {
        Font::PreparedFontString result;
        if (isFocused())
        {
            result = t.font->prepare(value + "_", 64, text_size < 0 ? t.size : text_size, getRenderSize(), Alignment::TopLeft);
            for(auto d : result.data)
                if (d.string_offset == cursor)
                    result.data.back().position = d.position;
        }
        else
        {
            result = t.font->prepare(value, 64, text_size < 0 ? t.size : text_size, getRenderSize(), Alignment::TopLeft);
        }
        render_data.mesh = result.create();
        render_data.texture = t.font->getTexture(64);
        texture_revision = render_data.texture->getRevision();
    }
    render_data.color = t.color;
}

void TextArea::onUpdate(float delta)
{
    if (render_data.texture && render_data.texture->getRevision() != texture_revision)
        markRenderDataOutdated();
    Widget::onUpdate(delta);
}

bool TextArea::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    if (isFocused())
    {
        const ThemeData::StateData& t = theme->states[int(getState())];
        if (t.font)
        {
            Font::PreparedFontString result = t.font->prepare(value + "_", 64, text_size < 0 ? t.size : text_size, getRenderSize(), Alignment::TopLeft);
            unsigned int n;
            for(n=0; n<result.data.size(); n++)
            {
                auto& d = result.data[n];
                if (d.position.y < position.y)
                    break;
            }
            float line_y = result.data[n].position.y;
            for(; n<result.data.size(); n++)
            {
                auto& d = result.data[n];
                if (d.position.x > position.x)
                    break;
                if (d.position.y < line_y)
                    break;
                cursor = d.string_offset;
            }
        }
    }else{
        cursor = value.length();
    }
    return true;
}

void TextArea::onPointerDrag(Vector2d position, int id)
{
}

void TextArea::onPointerUp(Vector2d position, int id)
{
}

void TextArea::onTextInput(const string& text)
{
    value = value.substr(0, cursor) + text + value.substr(cursor);
    cursor += text.length();
    markRenderDataOutdated();
}

void TextArea::onTextInput(TextInputEvent e)
{
    switch(e)
    {
    case TextInputEvent::Left: if (cursor > 0) cursor -= 1; break;
    case TextInputEvent::Right: if (cursor < int(value.length())) cursor += 1; break;
    case TextInputEvent::Up: break;
    case TextInputEvent::Down: break;
    case TextInputEvent::LineStart: cursor = 0; break;
    case TextInputEvent::LineEnd: cursor = value.length(); break;
    case TextInputEvent::TextStart: cursor = 0; break;
    case TextInputEvent::TextEnd: cursor = value.length(); break;
    case TextInputEvent::Delete:
        value = value.substr(0, cursor) + value.substr(cursor + 1);
        break;
    case TextInputEvent::Backspace:
        if (cursor > 0)
        {
            value = value.substr(0, cursor - 1) + value.substr(cursor);
            cursor -= 1;
        }
        break;
    case TextInputEvent::Return:
        onTextInput("\n");
        break;
    }
    markRenderDataOutdated();
}

};//namespace gui
};//namespace sp
