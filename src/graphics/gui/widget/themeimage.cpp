#include <sp2/graphics/gui/widget/themeimage.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

ThemeImage::ThemeImage(P<Widget> parent, const string& theme_style_name)
: Widget(parent)
{
    loadThemeStyle(theme_style_name);
    setAttribute("order", "-2");
}

void ThemeImage::setFlags(int flags)
{
    if (this->flags != flags)
    {
        this->flags = flags;
        markRenderDataOutdated();
    }
}

void ThemeImage::updateRenderData()
{
    updateRenderDataToThemeImage(flags);
}

}//namespace gui
}//namespace sp
