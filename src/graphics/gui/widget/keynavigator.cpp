#include <sp2/graphics/gui/widget/keynavigator.h>
#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/widget/togglebutton.h>
#include <sp2/graphics/gui/widget/slider.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/io/keybinding.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

KeyNavigator::KeyNavigator(P<Widget> parent)
: Widget(parent)
{
    loadThemeStyle("navigator");
    up = io::Keybinding::getByName("UP");
    down = io::Keybinding::getByName("DOWN");
    left = io::Keybinding::getByName("LEFT");
    right = io::Keybinding::getByName("RIGHT");
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
    else if (key == "left")
    {
        left = io::Keybinding::getByName(value);
    }
    else if (key == "right")
    {
        right = io::Keybinding::getByName(value);
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
    updateRenderDataToThemeImage();
}

void KeyNavigator::onUpdate(float delta)
{
    Widget::onUpdate(delta);

    if (skip)
    {
        skip = false;
        return;
    }
    if (!isEnabled())
        return;

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
    if (left && left->getDown())
    {
        if (!isVisible())
        {
            show();
            return;
        }
        P<Slider> slider = getParent();
        if (slider)
        {
            slider->setValue(slider->getValue() - (slider->getMax() - slider->getMin()) * 0.1f, true);
        }
    }
    if (right && right->getDown())
    {
        if (!isVisible())
        {
            show();
            return;
        }
        P<Slider> slider = getParent();
        if (slider)
            slider->setValue(slider->getValue() + (slider->getMax() - slider->getMin()) * 0.1f, true);
    }
    if (select && select->getDown() && isVisible())
    {
        select_down_widget = getParent();
    }
    if (select && select->getUp())
    {
        if (!isVisible())
        {
            show();
            return;
        }

        if (getParent() == select_down_widget)
        {
            if (P<Button> button = getParent())
            {
                button->onPointerDown(io::Pointer::Button::Left, Vector2d(0, 0), -2);
                button->onPointerUp(Vector2d(0, 0), -2);
            } else if (P<ToggleButton> tbutton = getParent())
            {
                tbutton->onPointerDown(io::Pointer::Button::Left, Vector2d(0, 0), -2);
                tbutton->onPointerUp(Vector2d(0, 0), -2);
            }
        }
    }
}

P<Widget> KeyNavigator::findFirstTarget(P<Widget> w)
{
    if (!w) return nullptr;
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

P<Widget> KeyNavigator::findNextTarget(P<Widget> w, P<Widget> after)
{
    if (!w)
    {
        w = getParent();
        while(P<Widget>(w->getParent()))
            w = w->getParent();
        return findFirstTarget(w);
    }
    bool found = !after;
    for(auto child : w->getChildren())
    {
        if (child == after)
            found = true;
        else if (found && canTarget(child))
            return child;
        else if (found)
            return findNextTarget(child, nullptr);
    }
    return findNextTarget(w->getParent(), w);
}

bool KeyNavigator::canTarget(P<Widget> w)
{
    if (!w->isVisible())
        return false;
    if (P<Button>(w))
        return true;
    if (P<ToggleButton>(w))
        return true;
    if (P<Slider>(w))
        return true;
    return false;
}

}//namespace gui
}//namespace sp
