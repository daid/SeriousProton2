#include <sp2/graphics/gui/widget/listbox.h>
#include <sp2/graphics/gui/widget/slider.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/stringutil/convert.h>
#include <sp2/io/keybinding.h>
#include <sp2/engine.h>


namespace sp {
namespace gui {

SP_REGISTER_WIDGET("listbox", Listbox);

Listbox::Listbox(P<Widget> parent)
: Widget(parent)
{
    loadThemeStyle("listbox.background");
    text_theme = Theme::getTheme("default")->getStyle("listbox.forground");

    slider = new Slider(this);
    slider->setAttribute("alignment", "right");
    slider->setAttribute("fill_height", "true");
    slider->setValue(0.0);
    slider->setEventCallback([this](sp::Variant v)
    {
        markRenderDataOutdated();
    });
}

void Listbox::setAttribute(const string& key, const string& value)
{
    if (key == "text_size" || key == "text.size")
    {
        text_size = stringutil::convert::toFloat(value);
    }
    else if (key == "style" || key == "theme_data")
    {
        Widget::setAttribute("style", value + ".background");
        text_theme = Theme::getTheme("default")->getStyle(value + ".forground");
    }
    else if (key == "items")
    {
        items = value.split(",");
        for(auto& e : items)
            e = e.strip();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Listbox::onUpdate(float delta)
{
    Widget::onUpdate(delta);
    for(auto node : text_nodes)
        node->render_data.type = render_data.type;
}

void Listbox::updateRenderData()
{
    const ThemeStyle::StateStyle& bt = theme->states[int(getState())];
    const ThemeStyle::StateStyle& ft = text_theme->states[int(getState())];

    double entry_height = (text_size > 0.0 ? text_size : ft.size) * 2.0;
    render_data.shader = Shader::get("internal:basic.shader");
    render_data.texture = bt.texture;
    if (bt.texture)
    {
        Rect2d selection_area(0, getRenderSize().y + slider->getValue() - entry_height - active_index * entry_height, getRenderSize().x - bt.size, entry_height);
        render_data.mesh = createStretchedHV(selection_area, Rect2d(0, 0, getRenderSize().x, getRenderSize().y), bt.size);
    }
    else
    {
        render_data.mesh = nullptr;
    }
    render_data.color = bt.color;

    slider->layout.size.x = bt.size;

    if (ft.font)
    {
        auto node_it = text_nodes.begin();
        int index = 0;
        for(const auto& item : items)
        {
            sp::P<sp::Node> node;
            if (node_it == text_nodes.end())
            {
                node = new sp::Node(this);
                text_nodes.add(node);
            }
            else
            {
                node = *node_it;
                ++node_it;
            }

            node->render_data.shader = render_data.shader;
            node->render_data.order = render_data.order + 1;
            auto text = ft.font->prepare(item, 32, text_size > 0.0 ? text_size : ft.size, Vector2d(getRenderSize().x - bt.size, entry_height), Alignment::Center, Font::FlagClip);
            double y = getRenderSize().y - entry_height - entry_height * index + slider->getValue();
            if (y < 0.0)
            {
                node->setPosition(sp::Vector2d(0, 0.0));
                for(auto& d : text.data)
                    d.position.y += y;
            }else if (y > getRenderSize().y - entry_height)
            {
                node->setPosition(sp::Vector2d(0, getRenderSize().y - entry_height));
                for(auto& d : text.data)
                    d.position.y += y - (getRenderSize().y - entry_height);
            }else{
                node->setPosition(sp::Vector2d(0, y));
            }
            node->render_data.mesh = text.create();
            node->render_data.color = ft.color;
            node->render_data.texture = ft.font->getTexture(32);
            index += 1;
        }

        slider->setRange(std::max(0.0, entry_height * items.size() - getRenderSize().y), 0);
    }
}

bool Listbox::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    if (isEnabled())
        playThemeSound(State::Normal);
    pointer_position[id] = position;
    return true;
}

void Listbox::onPointerDrag(Vector2d position, int id)
{
}

void Listbox::onPointerUp(Vector2d position, int id)
{
    if (position.x >= 0 && position.x <= getRenderSize().x && position.y >= 0 && position.y <= getRenderSize().y && isEnabled())
    {
        double offset = getRenderSize().y - position.y + slider->getValue();
        double entry_height = (text_size > 0.0 ? text_size : text_theme->states[int(getState())].size) * 2.0;
        setSelectedIndex(offset / entry_height);
        playThemeSound(State::Hovered);
        runCallback(active_index);
    }
}

void Listbox::clearItems()
{
    items.clear();
    active_index = 0;
    markRenderDataOutdated();
}

int Listbox::addItem(const string& label)
{
    items.push_back(label);
    markRenderDataOutdated();
    return items.size() - 1;
}

int Listbox::getSelectedIndex() const
{
    return active_index;
}

void Listbox::setSelectedIndex(int index)
{
    if (items.size() > 0)
    {
        while(index < 0)
            index += items.size();
        active_index = index % items.size();
    }
    else
    {
        active_index = 0;
    }
    markRenderDataOutdated();
}

}//namespace gui
}//namespace sp
