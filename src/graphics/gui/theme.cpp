#include <sp2/graphics/gui/theme.h>
#include <sp2/logging.h>

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
        return getData("");
    return getData(element.substr(0, n));
}

P<Theme> Theme::getTheme(string name)
{
    auto it = themes.find(name);
    if (it != themes.end())
        return it->second;
    LOG(Warning, "Theme", name, "not found. Falling back to default theme.");
    return getTheme("default");
}

};//!namespace gui
};//!namespace sp
