#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/io/keybinding.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

SP_REGISTER_WIDGET("button", Button);

Button::Button(P<Widget> parent)
: Widget(parent)
{
    loadThemeStyle("button.background");
    label = new Label(this, "button.forground");
    label->layout.fill_height = true;
    label->layout.fill_width = true;
    slave_widget = label;
}

void Button::setLabel(const string& value)
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
    else if (key == "style" || key == "theme_data")
    {
        Widget::setAttribute("style", value + ".background");
        label->setAttribute("style", value + ".forground");
    }
    else if (key == "keybinding")
    {
        keybinding = io::Keybinding::getByName(value);
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Button::onUpdate(float delta)
{
    Widget::onUpdate(delta);

    if (isVisible() && isEnabled() && keybinding)
    {
        if (keybinding->getDown())
        {
            playThemeSound(State::Normal);
            keybinding_down = true;
        }
        if (keybinding->getUp() && keybinding_down)
        {
            keybinding_down = false;
            playThemeSound(State::Hovered);
            runCallback(Variant());
        }
    }
}

void Button::updateRenderData()
{
    updateRenderDataToThemeImage();
}

bool Button::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    if (isEnabled())
        playThemeSound(State::Normal);
    return true;
}

void Button::onPointerUp(Vector2d position, int id)
{
    if (position.x >= 0 && position.x <= getRenderSize().x && position.y >= 0 && position.y <= getRenderSize().y && isEnabled())
    {
        playThemeSound(State::Hovered);
        runCallback(Variant());
    }
}

}//namespace gui
}//namespace sp
