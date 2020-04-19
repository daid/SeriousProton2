#include <sp2/graphics/gui/widget/keynavigator.h>
#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/io/keybinding.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

SP_REGISTER_WIDGET("keynavigator", KeyNavigator);

KeyNavigator::KeyNavigator(P<Widget> parent)
: Widget(parent)
{
    loadThemeStyle("navigator");
    up = io::Keybinding::getByName("UP");
    down = io::Keybinding::getByName("DOWN");
    select = io::Keybinding::getByName("START");
    hide();
}

void KeyNavigator::setAttribute(const string& key, const string& value)
{
    if (key == "up")
    {
        up = io::Keybinding::getByName(value);
    }
    else if (key == "down")
    {
        down = io::Keybinding::getByName(value);
    }
    else if (key == "select")
    {
        select = io::Keybinding::getByName(value);
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void KeyNavigator::updateRenderData()
{
    const sp::gui::ThemeStyle::StateStyle& t = theme->states[int(getState())];

    render_data.shader = sp::Shader::get("internal:basic.shader");
    render_data.mesh = createStretchedHV(getRenderSize(), t.size);
    render_data.texture = t.texture;
    render_data.color = t.color;
}

void KeyNavigator::onUpdate(float delta)
{
    sp::gui::Widget::onUpdate(delta);

    if (skip)
    {
        skip = false;
        return;
    }

    if (up && up->getDown())
    {
        if (!isVisible())
        {
            show();
            return;
        }
        auto next = findNextTarget(getParent(), nullptr);
        while(next && findNextTarget(next, nullptr) != getParent())
            next = findNextTarget(next, nullptr);
        if (next)
            setParent(next);
        skip = true;
    }
    if (down && down->getDown())
    {
        if (!isVisible())
        {
            show();
            return;
        }

        auto next = findNextTarget(getParent(), nullptr);
        if (next)
            setParent(next);
        skip = true;
    }
    if (select && select->getDown())
    {
        if (!isVisible())
        {
            show();
            return;
        }

        sp::P<sp::gui::Widget> parent = getParent();
        parent->onPointerDown(sp::io::Pointer::Button::Left, sp::Vector2d(0, 0), -2);
        parent->onPointerUp(sp::Vector2d(0, 0), -2);
    }
}

sp::P<sp::gui::Widget> KeyNavigator::findFirstTarget(sp::P<sp::gui::Widget> w)
{
    if (canTarget(w))
        return w;
    for(auto child : w->getChildren())
    {
        auto result = findFirstTarget(child);
        if (result)
            return result;
    }
    return nullptr;
}

sp::P<sp::gui::Widget> KeyNavigator::findNextTarget(sp::P<sp::gui::Widget> w, sp::P<sp::gui::Widget> after)
{
    if (!w)
    {
        w = getParent();
        while(sp::P<sp::gui::Widget>(w->getParent()))
            w = w->getParent();
        return findFirstTarget(w);
    }
    bool found = false;
    for(auto child : w->getChildren())
    {
        if (child == after)
            found = true;
        else if (found && canTarget(child))
            return child;
    }
    return findNextTarget(w->getParent(), w);
}

bool KeyNavigator::canTarget(sp::P<sp::gui::Widget> w)
{
    if (sp::P<sp::gui::Button>(w))
        return true;
    return false;
}

}//namespace gui
}//namespace sp
