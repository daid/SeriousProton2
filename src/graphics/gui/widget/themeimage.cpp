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
    orientation = Orientation::Auto;
    setAttribute("order", "-2");
}

void ThemeImage::setOrientation(Orientation orientation)
{
    if (this->orientation != orientation)
    {
        this->orientation = orientation;
        markRenderDataOutdated();
    }
}

void ThemeImage::updateRenderData()
{
    const ThemeStyle::StateStyle& t = theme->states[int(getState())];
    
    render_data.shader = Shader::get("internal:basic.shader");
    switch(orientation)
    {
    case Orientation::Auto:
        render_data.mesh = createStretched(getRenderSize());
        break;
    case Orientation::Horizontal:
        render_data.mesh = createStretchedH(getRenderSize());
        break;
    case Orientation::Vertical:
        render_data.mesh = createStretchedV(getRenderSize());
        break;
    }
    render_data.texture = t.texture;
    render_data.color = t.color;
}

}//namespace gui
}//namespace sp
