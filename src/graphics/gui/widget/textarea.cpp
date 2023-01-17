#include <sp2/graphics/gui/widget/textarea.h>
#include <sp2/graphics/gui/widget/slider.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/graphics/meshbuilder.h>
#include <sp2/io/clipboard.h>
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

    setAttribute("multiline", "true");
}

void TextArea::setAttribute(const string& key, const string& value)
{
    if (key == "text")
    {
        this->value = value;
        selection_start = std::min(int(value.length()), selection_start);
        selection_end = std::min(int(value.length()), selection_end);
        markRenderDataOutdated();
    }
    else if (key == "text_size" || key == "text.size")
    {
        text_size = stringutil::convert::toFloat(value);
        if (vertical_scroll)
            vertical_scroll->setSize(text_size, 0);
        markRenderDataOutdated();
    }
    else if (key == "multiline")
    {
        multiline = stringutil::convert::toBool(value);
        if (!multiline)
        {
            vertical_scroll.destroy();
            horizontal_scroll.destroy();
        }
        else if (!vertical_scroll)
        {
            vertical_scroll = new Slider(this);
            vertical_scroll->setSize(theme->states[int(getState())].size, 0);
            vertical_scroll->layout.alignment = Alignment::TopRight;
            vertical_scroll->layout.fill_height = true;
            vertical_scroll->setRange(100, 0);
            vertical_scroll->setEventCallback([this](Variant v) { markRenderDataOutdated(); });

            horizontal_scroll = new Slider(this);
            horizontal_scroll->setSize(0, theme->states[int(getState())].size);
            horizontal_scroll->layout.margin.right = theme->states[int(getState())].size;
            horizontal_scroll->layout.alignment = Alignment::BottomLeft;
            horizontal_scroll->layout.fill_width = true;
            horizontal_scroll->setRange(100, 0);
            horizontal_scroll->setEventCallback([this](Variant v) { markRenderDataOutdated(); });
        }
        markRenderDataOutdated();
    }
    else if (key == "readonly")
    {
        readonly = stringutil::convert::toBool(value);
    }
    else if (key == "theme_data" || key == "style")
    {
        Widget::setAttribute(key, value);
        if (vertical_scroll) vertical_scroll->setSize(theme->states[int(getState())].size, 0);
        if (horizontal_scroll) horizontal_scroll->setSize(0, theme->states[int(getState())].size);
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
        Font::PreparedFontString result = t.font->prepare(value, 64, t_size, getRenderSize(), multiline ? Alignment::TopLeft : Alignment::Left, Font::FlagClip);
        processPreparedFontString(result);
        render_data.mesh = result.create();
        render_data.texture = t.font->getTexture(64);
        texture_revision = render_data.texture->getRevision();

        cursor_widget->setVisible(isFocused());
        if (isFocused())
        {
            MeshBuilder mb;
            float start_x = -1.0f;
            int selection_min = std::min(selection_start, selection_end);
            int selection_max = std::max(selection_start, selection_end);
            for(auto d : result.data)
            {
                if (d.string_offset == selection_min)
                {
                    start_x = d.position.x;
                }
                if ((d.string_offset == selection_max) || (d.char_code == 0 && start_x > -1.0f))
                {
                    float end_x = d.position.x;
                    float start_y = d.position.y;
                    float end_y = start_y + t_size;
                    if (end_y >= 0.0 && start_y <= getRenderSize().y)
                    {
                        start_y = std::max(0.0f, start_y);
                        end_x = std::min(float(getRenderSize().x), end_x);
                        end_y = std::min(float(getRenderSize().y), end_y);
                        if (end_x != start_x)
                        {
                            mb.addQuad(
                                Vector3f(start_x, end_y, 0),
                                Vector3f(start_x, start_y, 0),
                                Vector3f(end_x, end_y, 0),
                                Vector3f(end_x, start_y, 0));
                        }
                    }
                    if (d.string_offset == selection_max)
                        start_x = -1.0f;
                    else
                        start_x = 0.0f;
                }
                if (d.string_offset == selection_end)
                {
                    float start_y = d.position.y;
                    float end_y = start_y + t_size;
                    if (end_y < 0.0)
                        continue;
                    if (start_y > getRenderSize().y)
                        continue;
                    start_y = std::max(0.0f, start_y);
                    end_y = std::min(float(getRenderSize().y), end_y);

                    mb.addQuad(
                        Vector3f(d.position.x - t_size * 0.05f, end_y, 0),
                        Vector3f(d.position.x - t_size * 0.05f, start_y, 0),
                        Vector3f(d.position.x + t_size * 0.05f, end_y, 0),
                        Vector3f(d.position.x + t_size * 0.05f, start_y, 0));
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

void TextArea::processPreparedFontString(Font::PreparedFontString& pfs)
{
    const ThemeStyle::StateStyle& t = theme->states[int(getState())];
    auto text_area_size = pfs.getUsedAreaSize();
    if (vertical_scroll)
    {
        vertical_scroll->setRange(std::max(0.0, text_area_size.y - getRenderSize().y + t.size), 0);
        vertical_scroll->setVisible(text_area_size.y > getRenderSize().y - t.size);
        for(auto& data : pfs.data)
            data.position.y += vertical_scroll->getValue();
    }
    if (horizontal_scroll)
    {
        horizontal_scroll->setRange(0, std::max(0.0, text_area_size.x - getRenderSize().x + t.size));
        horizontal_scroll->setVisible(text_area_size.x > getRenderSize().x - t.size);
        for(auto& data : pfs.data)
            data.position.x -= horizontal_scroll->getValue();
        if (vertical_scroll->isVisible())
            horizontal_scroll->layout.margin.right = vertical_scroll->layout.size.x;
        else
            horizontal_scroll->layout.margin.right = 0;
    }
}

void TextArea::onUpdate(float delta)
{
    if (render_data.texture && render_data.texture->getRevision() != texture_revision)
        markRenderDataOutdated();
    Widget::onUpdate(delta);
}

bool TextArea::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    selection_start = getTextOffsetForPosition(position);
    selection_end = selection_start;
    scrollIntoView(selection_end);
    markRenderDataOutdated();
    return true;
}

void TextArea::onPointerDrag(Vector2d position, int id)
{
    selection_end = getTextOffsetForPosition(position);
    scrollIntoView(selection_end);
    markRenderDataOutdated();
}

void TextArea::onPointerUp(Vector2d position, int id)
{
}

bool TextArea::onWheelMove(sp::Vector2d position, sp::io::Pointer::Wheel direction)
{
    if (!multiline) return false;
    const ThemeStyle::StateStyle& t = theme->states[int(getState())];
    switch(direction) {
    case sp::io::Pointer::Wheel::Up: vertical_scroll->setValue(vertical_scroll->getValue() - t.size, true); break;
    case sp::io::Pointer::Wheel::Down: vertical_scroll->setValue(vertical_scroll->getValue() + t.size, true); break;
    case sp::io::Pointer::Wheel::Left: horizontal_scroll->setValue(horizontal_scroll->getValue() - t.size, true); break;
    case sp::io::Pointer::Wheel::Right: horizontal_scroll->setValue(horizontal_scroll->getValue() + t.size, true); break;
    }
    return true;
}

void TextArea::onTextInput(const string& text)
{
    if (readonly)
        return;
    value = value.substr(0, std::min(selection_start, selection_end)) + text + value.substr(std::max(selection_start, selection_end));
    selection_end = selection_start = std::min(selection_start, selection_end) + text.length();
    scrollIntoView(selection_end);
    markRenderDataOutdated();
}

void TextArea::onTextInput(TextInputEvent e)
{
    switch(e)
    {
    case TextInputEvent::Left:
    case TextInputEvent::LeftWithSelection:
        if (selection_end > 0)
            selection_end -= 1;
        if (e != TextInputEvent::LeftWithSelection)
            selection_start = selection_end;
        scrollIntoView(selection_end);
        break;
    case TextInputEvent::Right:
    case TextInputEvent::RightWithSelection:
        if (selection_end < int(value.length()))
            selection_end += 1;
        if (e != TextInputEvent::RightWithSelection)
            selection_start = selection_end;
        scrollIntoView(selection_end);
        break;
    case TextInputEvent::WordLeft:
    case TextInputEvent::WordLeftWithSelection:
        if (selection_end > 0)
            selection_end -= 1;
        while (selection_end > 0 && !isspace(value[selection_end - 1]))
            selection_end -= 1;
        if (e != TextInputEvent::WordLeftWithSelection)
            selection_start = selection_end;
        scrollIntoView(selection_end);
        break;
    case TextInputEvent::WordRight:
    case TextInputEvent::WordRightWithSelection:
        while (selection_end < int(value.length()) && !isspace(value[selection_end]))
            selection_end += 1;
        if (selection_end < int(value.length()))
            selection_end += 1;
        if (e != TextInputEvent::WordRightWithSelection)
            selection_start = selection_end;
        scrollIntoView(selection_end);
        break;
    case TextInputEvent::Up:
    case TextInputEvent::UpWithSelection:{
        int end_of_line = value.substr(0, selection_end).rfind("\n");
        if (end_of_line < 0)
            return;
        int start_of_line = value.substr(0, end_of_line).rfind("\n") + 1;
        int offset = selection_end - end_of_line - 1;
        int line_length = end_of_line - start_of_line;
        selection_end = start_of_line + std::min(line_length, offset);
        if (e != TextInputEvent::UpWithSelection)
            selection_start = selection_end;
        scrollIntoView(selection_end);
        }break;
    case TextInputEvent::Down:
    case TextInputEvent::DownWithSelection:{
        int start_of_current_line = value.substr(0, selection_end).rfind("\n") + 1;
        int end_of_current_line = value.find("\n", selection_end);
        if (end_of_current_line < 0)
            return;
        int end_of_end_line = value.find("\n", end_of_current_line + 1);
        if (end_of_end_line == -1)
            end_of_end_line = value.length();
        int offset = selection_end - start_of_current_line;
        selection_end = end_of_current_line + 1 + std::min(offset, end_of_end_line - (end_of_current_line + 1));
        if (e != TextInputEvent::DownWithSelection)
            selection_start = selection_end;
        scrollIntoView(selection_end);
        }break;
    case TextInputEvent::LineStart:
    case TextInputEvent::LineStartWithSelection:
        selection_end = value.substr(0, selection_end).rfind("\n") + 1;
        if (e != TextInputEvent::LineStartWithSelection)
            selection_start = selection_end;
        scrollIntoView(selection_end);
        break;
    case TextInputEvent::LineEnd:
    case TextInputEvent::LineEndWithSelection:
        selection_end = value.find("\n", selection_start);
        if (selection_end == -1)
            selection_end = value.length();
        if (e != TextInputEvent::LineEndWithSelection)
            selection_start = selection_end;
        scrollIntoView(selection_end);
        break;
    case TextInputEvent::TextStart:
    case TextInputEvent::TextStartWithSelection:
        selection_end = 0;
        if (e != TextInputEvent::TextStartWithSelection)
            selection_start = selection_end;
        scrollIntoView(selection_end);
        break;
    case TextInputEvent::TextEnd:
    case TextInputEvent::TextEndWithSelection:
        selection_end = value.length();
        if (e != TextInputEvent::TextEndWithSelection)
            selection_start = selection_end;
        scrollIntoView(selection_end);
        break;
    case TextInputEvent::SelectAll:
        selection_end = 0;
        selection_start = value.length();
        scrollIntoView(selection_end);
        break;
    case TextInputEvent::Delete:
        if (readonly)
            return;
        if (selection_start != selection_end)
            value = value.substr(0, std::min(selection_start, selection_end)) + value.substr(std::max(selection_start, selection_end));
        else
            value = value.substr(0, selection_start) + value.substr(selection_start + 1);
        selection_start = selection_end = std::min(selection_start, selection_end);
        scrollIntoView(selection_end);
        break;
    case TextInputEvent::Backspace:
        if (readonly)
            return;
        if (selection_start != selection_end)
        {
            onTextInput(TextInputEvent::Delete);
            return;
        }
        else if (selection_start > 0)
        {
            value = value.substr(0, selection_start - 1) + value.substr(selection_start);
            selection_start -= 1;
            selection_end = selection_start;
            scrollIntoView(selection_end);
        }
        break;
    case TextInputEvent::Indent:
        if (readonly)
            return;
        if (selection_start == selection_end)
        {
            int start_of_line = value.substr(0, selection_end).rfind("\n") + 1;
            int offset = selection_end - start_of_line;
            int add = 4 - (offset % 4);
            onTextInput(string(" ") * add);
        }
        else
        {
            int start_of_line = value.substr(0, std::min(selection_start, selection_end)).rfind("\n") + 1;
            auto data = value.substr(start_of_line, std::max(selection_start, selection_end));
            data = "    " + data.replace("\n", "\n    ");
            int extra_length = data.length() - (std::max(selection_start, selection_end) - start_of_line) - 4;
            value = value.substr(0, start_of_line) + data + value.substr(std::max(selection_start, selection_end));

            if (start_of_line != selection_start)
                selection_start += 4;
            if (start_of_line != selection_end)
                selection_end += 4;
            if (selection_start > selection_end)
                selection_start += extra_length;
            else
                selection_end += extra_length;
        }
        break;
    case TextInputEvent::Unindent:
        if (readonly)
            return;
        if (selection_start == selection_end)
        {
        }
        else
        {
            int start_of_line = value.substr(0, std::min(selection_start, selection_end)).rfind("\n") + 1;
            auto data = value.substr(start_of_line, std::max(selection_start, selection_end));
            for(int n=0; n<4; n++)
            {
                if (data.startswith(" "))
                    data = data.substr(1);
                data = data.replace("\n ", "\n");
            }
            int removed_length = (std::max(selection_start, selection_end) - start_of_line) - data.length();
            value = value.substr(0, start_of_line) + data + value.substr(std::max(selection_start, selection_end));

            if (selection_start > selection_end)
                selection_start -= removed_length;
            else
                selection_end -= removed_length;
        }
        break;
    case TextInputEvent::Return:
        if (readonly)
            return;
        if (multiline)
            onTextInput("\n");
        else
            runCallback(value);
        break;
    case TextInputEvent::Copy:
        io::Clipboard::set(value.substr(std::min(selection_start, selection_end), std::max(selection_start, selection_end)));
        break;
    case TextInputEvent::Paste:
        if (readonly)
            return;
        onTextInput(io::Clipboard::get());
        break;
    case TextInputEvent::Cut:
        io::Clipboard::set(value.substr(std::min(selection_start, selection_end), std::max(selection_start, selection_end)));
        if (readonly)
            return;
        if (selection_start != selection_end)
            onTextInput(TextInputEvent::Delete);
        break;
    }
    markRenderDataOutdated();
}

const string& TextArea::getValue() const
{
    return value;
}

void TextArea::scrollIntoView(int offset)
{
    if ((vertical_scroll && vertical_scroll->getMin() > 0.0f) || (horizontal_scroll && horizontal_scroll->getMax() > 0.0f)) {
        const ThemeStyle::StateStyle& t = theme->states[int(getState())];
        if (!t.font) return;        
        auto pfs = t.font->prepare(value, 64, text_size < 0 ? t.size : text_size, getRenderSize(), multiline ? Alignment::TopLeft : Alignment::Left);
        auto text_area_size = pfs.getUsedAreaSize();
        for(auto g : pfs.data) {
            if (g.string_offset == offset) {
                vertical_scroll->setRange(std::max(0.0, text_area_size.y - getRenderSize().y + t.size), 0);
                horizontal_scroll->setRange(0, std::max(0.0, text_area_size.x - getRenderSize().x + t.size));

                float vmin = -g.position.y;
                float vmax = getRenderSize().y - g.position.y - t.size;
                if (horizontal_scroll->isVisible())
                    vmin += t.size;
                vertical_scroll->setValue(std::clamp(vertical_scroll->getValue(), vmin, vmax));

                float hmin = g.position.x - getRenderSize().x;
                float hmax = g.position.x;
                if (vertical_scroll->isVisible())
                    hmin += t.size;
                horizontal_scroll->setValue(std::clamp(horizontal_scroll->getValue(), hmin, hmax));
                break;
            }
        }
    }
}

int TextArea::getTextOffsetForPosition(Vector2d position)
{
    int result = value.size();
    if (vertical_scroll)
        position.y -= vertical_scroll->getValue();
    if (horizontal_scroll)
        position.x += horizontal_scroll->getValue();
    const ThemeStyle::StateStyle& t = theme->states[int(getState())];
    if (t.font)
    {
        auto pfs = t.font->prepare(value, 64, text_size < 0 ? t.size : text_size, getRenderSize(), multiline ? Alignment::TopLeft : Alignment::Left);
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
