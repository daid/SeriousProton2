#include <sp2/graphics/gui/widget/textfield.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("textfield", TextField);

TextField::TextField(P<Widget> parent)
: Widget(parent)
{
    loadThemeData("textfield");
    text_size = -1;
    texture_revision = -1;
    setFocusable(true);
    
    setAttribute("order", "-1");
    
    cursor_widget = new Widget(P<Widget>(this));
}

void TextField::setAttribute(const string& key, const string& value)
{
    if (key == "text" || key == "value")
    {
        this->value = value;
        markRenderDataOutdated();
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

void TextField::updateRenderData()
{
    const ThemeData::StateData& t = theme->states[int(getState())];

    render_data.shader = Shader::get("internal:basic.shader");
    if (t.font)
    {
        float t_size = text_size < 0 ? t.size : text_size;
    
        Font::PreparedFontString result;
        result = t.font->prepare(value + " ", 64, t_size, getRenderSize(), Alignment::Left);
        render_data.mesh = result.create();
        render_data.texture = t.font->getTexture(64);
        texture_revision = render_data.texture->getRevision();

        cursor_widget->setVisible(isFocused());
        if (isFocused())
        {
            float start_x = 0;
            float end_x = 0;
            for(auto d : result.data)
            {
                if (d.string_offset == selection_start)
                    start_x = d.position.x;
                if (d.string_offset == selection_end)
                    end_x = d.position.x;
            }
            float width = std::max(end_x - start_x, t_size * 0.1f);
            cursor_widget->setPosition(start_x + width / 2.0, result.data.front().position.y, Alignment::Left);
            cursor_widget->render_data.shader = Shader::get("internal:color.shader");
            cursor_widget->render_data.mesh = MeshData::createQuad(sp::Vector2f(width, t_size));
            cursor_widget->render_data.color = t.color;
            if (start_x != end_x)
                cursor_widget->render_data.color.a = 0.5;
        }
    }
    render_data.color = t.color;
}

void TextField::onUpdate(float delta)
{
    if (render_data.texture && render_data.texture->getRevision() != texture_revision)
        markRenderDataOutdated();
    Widget::onUpdate(delta);
}

bool TextField::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    selecting_start_offset = getTextOffsetForPosition(position);
    selection_start = selecting_start_offset;
    selection_end = selecting_start_offset;
    markRenderDataOutdated();
    return true;
}

void TextField::onPointerDrag(Vector2d position, int id)
{
    int new_offset = getTextOffsetForPosition(position);
    selection_start = std::min(selecting_start_offset, new_offset);
    selection_end = std::max(selecting_start_offset, new_offset);
    markRenderDataOutdated();
}

void TextField::onPointerUp(Vector2d position, int id)
{
}

void TextField::onTextInput(const string& text)
{
    value = value.substr(0, selection_start) + text + value.substr(selection_end);
    selection_start += text.length();
    selection_end = selection_start;
    markRenderDataOutdated();
}

void TextField::onTextInput(TextInputEvent e)
{
    //SP2TODO: Handle proper unicode/utf-8 when the font does this as well.
    switch(e)
    {
    case TextInputEvent::Left: if (selection_start > 0) selection_start -= 1; selection_end = selection_start; break;
    case TextInputEvent::Right: if (selection_start < int(value.length())) selection_start += 1; selection_end = selection_start; break;
    case TextInputEvent::Up: break;
    case TextInputEvent::Down: break;
    case TextInputEvent::LineStart: selection_start = 0; selection_end = selection_start; break;
    case TextInputEvent::LineEnd: selection_start = value.length(); selection_end = selection_start; break;
    case TextInputEvent::TextStart: selection_start = 0; selection_end = selection_start; break;
    case TextInputEvent::TextEnd: selection_start = value.length(); selection_end = selection_start; break;
    case TextInputEvent::Delete:
        if (selection_start != selection_end)
            value = value.substr(0, selection_start) + value.substr(selection_end);
        else
            value = value.substr(0, selection_start) + value.substr(selection_start + 1);
        selection_end = selection_start;
        break;
    case TextInputEvent::Backspace:
        if (selection_start != selection_end)
        {
            value = value.substr(0, selection_start) + value.substr(selection_end);
            selection_end = selection_start;
        }
        else if (selection_start > 0)
        {
            value = value.substr(0, selection_start - 1) + value.substr(selection_start);
            selection_start -= 1;
            selection_end = selection_start;
        }
        break;
    case TextInputEvent::Return:
        runCallback(value);
        break;
    }
    markRenderDataOutdated();
}

int TextField::getTextOffsetForPosition(Vector2d position)
{
    int result_offset = 0;
    const ThemeData::StateData& t = theme->states[int(getState())];
    if (t.font)
    {
        Font::PreparedFontString result = t.font->prepare(value + " ", 64, text_size < 0 ? t.size : text_size, getRenderSize(), Alignment::Left);
        for(auto& d : result.data)
        {
            if (d.position.x <= position.x)
                result_offset = d.string_offset;
        }
    }
    return result_offset;
}

};//namespace gui
};//namespace sp
