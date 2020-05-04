#include <sp2/graphics/gui/widget/textarea.h>
#include <sp2/graphics/gui/widget/slider.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/graphics/meshbuilder.h>
#include <sp2/stringutil/convert.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("textarea", TextArea);

TextArea::TextArea(P<Widget> parent)
: Widget(parent)
{
    loadThemeStyle("textarea");
    text_size = -1;
    texture_revision = -1;
    setFocusable(true);
    
    setAttribute("order", "-1");

    cursor_widget = new Widget(P<Widget>(this));
    cursor_widget->layout.fill_width = true;
    cursor_widget->layout.fill_height = true;

    vertical_scroll = new Slider(this);
    vertical_scroll->setSize(theme->states[int(getState())].size, 0);
    vertical_scroll->layout.alignment = Alignment::Right;
    vertical_scroll->layout.fill_height = true;
    vertical_scroll->setRange(100, 0);
    vertical_scroll->setEventCallback([this](Variant v) { markRenderDataOutdated(); });
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
        vertical_scroll->setSize(text_size, 0);
        markRenderDataOutdated();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void TextArea::updateRenderData()
{
    const ThemeStyle::StateStyle& t = theme->states[int(getState())];

    render_data.shader = Shader::get("internal:basic.shader");
    if (t.font)
    {
        float t_size = text_size < 0 ? t.size : text_size;
        Font::PreparedFontString result = t.font->prepare(value, 64, t_size, getRenderSize(), Alignment::TopLeft, Font::FlagClip);
        vertical_scroll->setRange(std::max(0.0, result.getUsedAreaSize().y - getRenderSize().y), 0);
        for(auto& data : result.data)
            data.position.y += vertical_scroll->getValue();
        render_data.mesh = result.create();
        render_data.texture = t.font->getTexture(64);
        texture_revision = render_data.texture->getRevision();

        cursor_widget->setVisible(isFocused());
        if (isFocused())
        {
            MeshBuilder mb;
            float start_x = -1;
            for(auto d : result.data)
            {
                if (d.string_offset == selection_start)
                {
                    start_x = d.position.x;
                }
                if ((d.string_offset == selection_end) || (d.char_code == 0 && start_x > -1.0f))
                {
                    float end_x = d.position.x;
                    float start_y = d.position.y;
                    float end_y = start_y + t_size;
                    if (end_y < 0.0)
                        continue;
                    if (start_y > getRenderSize().y)
                        continue;
                    start_y = std::max(0.0f, start_y);
                    end_x = std::min(float(getRenderSize().x), end_x);
                    end_y = std::min(float(getRenderSize().y), end_y);
                    if (end_x == start_x)
                        end_x += t_size * 0.1f;
                    mb.addQuad(
                        Vector3f(start_x, end_y, 0),
                        Vector3f(start_x, start_y, 0),
                        Vector3f(end_x, end_y, 0),
                        Vector3f(end_x, start_y, 0));
                    if (d.string_offset == selection_end)
                        start_x = -1.0f;
                    else
                        start_x = 0.0f;
                }
            }
            cursor_widget->render_data.shader = Shader::get("internal:color.shader");
            cursor_widget->render_data.mesh = mb.create();
            cursor_widget->render_data.color = t.color;
            if (selection_start != selection_end)
                cursor_widget->render_data.color.a = 0.5;
        }
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
    selection_pointer_down = getTextOffsetForPosition(position);
    selection_start = selection_pointer_down;
    selection_end = selection_pointer_down;
    markRenderDataOutdated();
    return true;
}

void TextArea::onPointerDrag(Vector2d position, int id)
{
    int new_offset = getTextOffsetForPosition(position);
    selection_start = std::min(selection_pointer_down, new_offset);
    selection_end = std::max(selection_pointer_down, new_offset);
    markRenderDataOutdated();
}

void TextArea::onPointerUp(Vector2d position, int id)
{
}

void TextArea::onTextInput(const string& text)
{
    value = value.substr(0, selection_start) + text + value.substr(selection_end);
    selection_start += text.length();
    selection_end = selection_start;
    markRenderDataOutdated();
}

void TextArea::onTextInput(TextInputEvent e)
{
    switch(e)
    {
    case TextInputEvent::Left: if (selection_start > 0) selection_start -= 1; selection_end = selection_start; break;
    case TextInputEvent::Right: if (selection_start < int(value.length())) selection_start += 1; selection_end = selection_start; break;
    case TextInputEvent::WordLeft:
        if (selection_start > 0)
            selection_start -= 1;
        while (selection_start > 0 && !isspace(value[selection_start - 1]))
            selection_start -= 1;
        selection_end = selection_start;
        break;
    case TextInputEvent::WordRight:
        while (selection_start < int(value.length()) && !isspace(value[selection_start]))
            selection_start += 1;
        if (selection_start < int(value.length()))
            selection_start += 1;
        selection_end = selection_start;
        break;
    case TextInputEvent::Up:{
        int end_of_line = value.substr(0, selection_start).rfind("\n");
        if (end_of_line < 0) return;
        int start_of_line = value.substr(0, end_of_line).rfind("\n") + 1;
        int offset = selection_start - end_of_line - 1;
        int line_length = end_of_line - start_of_line;
        selection_start = start_of_line + std::min(line_length, offset);
        selection_end = selection_start;
        }break;
    case TextInputEvent::Down:{
        int start_of_current_line = value.substr(0, selection_start).rfind("\n") + 1;
        int end_of_current_line = value.find("\n", selection_start);
        if (end_of_current_line < 0)
            return;
        int end_of_end_line = value.find("\n", end_of_current_line + 1);
        if (end_of_end_line == -1)
            end_of_end_line = value.length();
        int offset = selection_start - start_of_current_line;
        selection_start = end_of_current_line + 1 + std::min(offset, end_of_end_line - (end_of_current_line + 1));
        selection_end = selection_start;
        }break;
    case TextInputEvent::LineStart:
        selection_start = value.substr(0, selection_start).rfind("\n") + 1;
        selection_end = selection_start;
        break;
    case TextInputEvent::LineEnd:
        selection_start = value.find("\n", selection_start);
        if (selection_start == -1)
            selection_start = value.length();
        selection_end = selection_start;
        break;
    case TextInputEvent::TextStart:
        selection_start = 0;
        selection_end = selection_start;
        break;
    case TextInputEvent::TextEnd:
        selection_start = value.length();
        selection_end = selection_start;
        break;
    case TextInputEvent::Delete:
        if (selection_start != selection_end)
            value = value.substr(0, selection_start) + value.substr(selection_end);
        else
            value = value.substr(0, selection_start) + value.substr(selection_end + 1);
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
        onTextInput("\n");
        break;
    }
    markRenderDataOutdated();
}

int TextArea::getTextOffsetForPosition(Vector2d position)
{
    int result = 0;
    position.y -= vertical_scroll->getValue();
    const ThemeStyle::StateStyle& t = theme->states[int(getState())];
    if (t.font)
    {
        Font::PreparedFontString pfs = t.font->prepare(value, 64, text_size < 0 ? t.size : text_size, getRenderSize(), Alignment::TopLeft);
        unsigned int n;
        for(n=0; n<pfs.data.size(); n++)
        {
            auto& d = pfs.data[n];
            if (d.position.y < position.y)
                break;
        }
        float line_y = pfs.data[n].position.y;
        for(; n<pfs.data.size(); n++)
        {
            auto& d = pfs.data[n];
            if (d.position.x > position.x)
                break;
            if (d.position.y < line_y)
                break;
            result = d.string_offset;
        }
    }
    return result;
}

}//namespace gui
}//namespace sp
