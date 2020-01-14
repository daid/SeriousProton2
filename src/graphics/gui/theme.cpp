#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/logging.h>
#include <sp2/io/keyValueTreeLoader.h>

namespace sp {
namespace gui {

std::map<string, P<Theme>> Theme::themes;

const ThemeStyle* Theme::getStyle(const string& element)
{
    auto it = styles.find(element);
    if (it != styles.end())
        return &it->second;
    int n = element.rfind(".");
    if (n == -1)
    {
        LOG(Warning, "Cannot find", element, "in theme", name);
        return getStyle("fallback");
    }
    return getStyle(element.substr(0, n));
}

P<Theme> Theme::getTheme(const string& name)
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

void Theme::loadTheme(const string& name, const string& resource_name)
{
    P<Theme> theme = new Theme(name);

    KeyValueTreePtr tree = io::KeyValueTreeLoader::load(resource_name);
    if (!tree)
        return;
    for(auto& it : tree->getFlattenNodesByIds())
    {
        std::map<string, string>& input = it.second;
        ThemeStyle::StateStyle global_style;
        ThemeStyle style;

        if (input["image"] == "")
            global_style.texture = nullptr;
        else
            global_style.texture = texture_manager.get(input["image"]);
        if (input.find("color") != input.end())
            global_style.color = Color::fromString(input["color"]);
        else
            global_style.color = Color(1, 1, 1);
        global_style.font = font_manager.get(input["font"]);
        global_style.size = stringutil::convert::toFloat(input["size"]);
        global_style.sound = input["sound"];
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
            style.states[n] = global_style;

            if (input.find("image." + postfix) != input.end())
            {
                if (input["image." + postfix] == "")
                    style.states[n].texture = nullptr;
                else
                    style.states[n].texture = texture_manager.get(input["image." + postfix]);
            }
            if (input.find("color." + postfix) != input.end())
                style.states[n].color = Color::fromString(input["color." + postfix]);
            if (input.find("font." + postfix) != input.end())
                style.states[n].font = font_manager.get(input["font." + postfix]);
            if (input.find("size." + postfix) != input.end())
                style.states[n].size = stringutil::convert::toFloat(input["size." + postfix]);
            if (input.find("sound." + postfix) != input.end())
                style.states[n].sound = input["sound." + postfix];
        }

        theme->styles[it.first] = style;
    }
}

Theme::Theme(const string& name)
: name(name)
{
    themes[name].destroy();
    themes[name] = this;

    ThemeStyle::StateStyle fallback_state;
    fallback_state.color = sp::Color(1, 0, 0);
    fallback_state.size = 12;
    fallback_state.font = nullptr;
    fallback_state.texture = nullptr;
    ThemeStyle fallback;
    for(unsigned int n=0; n<int(Widget::State::Count); n++)
        fallback.states[n] = fallback_state;
    styles["fallback"] = fallback;
}

Theme::~Theme()
{
}

}//namespace gui
}//namespace sp
