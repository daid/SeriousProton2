#include <sp2/graphics/gui/widget/togglebutton.h>
#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/io/keybinding.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

SP_REGISTER_WIDGET("togglebutton", ToggleButton);

ToggleButton::ToggleButton(P<Widget> parent)
: Widget(parent)
{
    style_name = "button";
    loadThemeStyle("button.background.inactive");
    label = new Label(this, "button.forground.inactive");
    label->layout.fill_height = true;
    label->layout.fill_width = true;
    slave_widget = label;
}

void ToggleButton::setLabel(const string& value)
{
    label->setLabel(value);
}

void ToggleButton::setActive(bool state)
{
    active = state;
    if (active)
    {
        Widget::setAttribute("style", style_name + ".background.active");
        label->setAttribute("style", style_name + ".forground.active");
    }
    else
    {
        Widget::setAttribute("style", style_name + ".background.inactive");
        label->setAttribute("style", style_name + ".forground.inactive");
    }
}

void ToggleButton::setAttribute(const string& key, const string& value)
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
        style_name = value;
        setActive(active);
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

void ToggleButton::onUpdate(float delta)
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
            setActive(!active);
            runCallback(Variant(active));
        }
    }
}

void ToggleButton::updateRenderData()
{
    updateRenderDataToThemeImage();
}

bool ToggleButton::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    if (isEnabled())
        playThemeSound(State::Normal);
    return true;
}

void ToggleButton::onPointerUp(Vector2d position, int id)
{
    if (position.x >= 0 && position.x <= getRenderSize().x && position.y >= 0 && position.y <= getRenderSize().y && isEnabled())
    {
        playThemeSound(State::Hovered);
        setActive(!active);
        runCallback(Variant(active));
    }
}

}//namespace gui
}//namespace sp
