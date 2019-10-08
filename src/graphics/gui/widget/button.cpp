#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

SP_REGISTER_WIDGET("button", Button);

Button::Button(P<Widget> parent)
: Widget(parent)
{
    loadThemeData("button.background");
    label = new Label(this, "button.forground");
    label->layout.fill_height = true;
    label->layout.fill_width = true;
    slave_widget = label;
}

void Button::setLabel(string value)
{
    label->setLabel(value);
}

void Button::setAttribute(const string& key, const string& value)
{
    if (key == "label" || key == "caption")
    {
        label->setLabel(value);
    }
    else if (key == "text_size" || key == "text.size")
    {
        label->setAttribute(key, value);
    }
    else if (key == "theme_data")
    {
        Widget::setAttribute("theme_data", value + ".background");
        label->setAttribute("theme_data", value + ".forground");
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Button::updateRenderData()
{
    const ThemeData::StateData& t = theme->states[int(getState())];

    render_data.shader = Shader::get("internal:basic.shader");
    render_data.texture = t.texture;
    if (t.texture)
        render_data.mesh = createStretched(getRenderSize());
    else
        render_data.mesh = nullptr;
    render_data.color = t.color;
}

bool Button::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    return true;
}

void Button::onPointerUp(Vector2d position, int id)
{
    if (position.x >= 0 && position.x <= getRenderSize().x && position.y >= 0 && position.y <= getRenderSize().y && isEnabled())
        runCallback(Variant());
}

}//namespace gui
}//namespace sp
