#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/logging.h>
#include <sp2/io/keyValueTreeLoader.h>

namespace sp {
namespace gui {

std::map<string, P<Theme>> Theme::themes;

const ThemeData* Theme::getData(string element)
{
    auto it = data.find(element);
    if (it != data.end())
        return &it->second;
    int n = element.rfind(".");
    if (n == -1)
    {
        LOG(Warning, "Cannot find", element, "in theme", name);
        return &fallback_data;
    }
    return getData(element.substr(0, n));
}

P<Theme> Theme::getTheme(string name)
{
    auto it = themes.find(name);
    if (it != themes.end())
        return it->second;
    if (name == "default")
    {
        LOG(Error, "Default theme not found. Most likely crashing now.");
        return nullptr;
    }
    LOG(Warning, "Theme", name, "not found. Falling back to [default] theme.");
    return getTheme("default");
}

void Theme::loadTheme(string name, string resource_name)
{
    P<Theme> theme = new Theme(name);
    for(unsigned int n=0; n<int(Widget::State::Count); n++)
    {
        theme->fallback_data.states[n].color = sp::Color::White;
    }

    P<KeyValueTree> tree = io::KeyValueTreeLoader::load(resource_name);
    for(auto& it : tree->getFlattenNodesByIds())
    {
        std::map<string, string>& input = it.second;
        ThemeData::StateData global_data;
        ThemeData data;

        if (input["image"] == "")
            global_data.texture = nullptr;
        else
            global_data.texture = textureManager.get(input["image"]);
        if (input.find("color") != input.end())
            global_data.color = Color::fromString(input["color"]);
        else
            global_data.color = Color::White;
        global_data.font = font_manager.get(input["font"]);
        global_data.text_size = stringutil::convert::toFloat(input["text.size"]);
        for(unsigned int n=0; n<int(Widget::State::Count); n++)
        {
            string postfix = "?";
            switch(Widget::State(n))
            {
            case Widget::State::Normal:
                postfix = "normal";
                break;
            case Widget::State::Disabled:
                postfix = "disabled";
                break;
            case Widget::State::Focused:
                postfix = "focused";
                break;
            case Widget::State::Hovered:
                postfix = "hovered";
                break;
            case Widget::State::Count:
                break;
            }
            data.states[n] = global_data;

            if (input.find("image." + postfix) != input.end())
            {
                if (input["image." + postfix] == "")
                    data.states[n].texture = nullptr;
                else
                    data.states[n].texture = textureManager.get(input["image." + postfix]);
            }
            if (input.find("color." + postfix) != input.end())
                data.states[n].color = Color::fromString(input["color." + postfix]);
            if (input.find("font." + postfix) != input.end())
                data.states[n].font = font_manager.get(input["font." + postfix]);
            if (input.find("text.size." + postfix) != input.end())
                data.states[n].text_size = stringutil::convert::toFloat(input["text.size." + postfix]);
        }

        theme->data[it.first] = data;
    }
    tree.destroy();
}

Theme::Theme(string name)
: name(name)
{
    themes[name] = this;
}

Theme::~Theme()
{
}

};//!namespace gui
};//!namespace sp
