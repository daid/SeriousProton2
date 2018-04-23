#include <sp2/graphics/gui/widget/themeimage.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

ThemeImage::ThemeImage(P<Widget> parent, string theme_data_name)
: Widget(parent)
{
    loadThemeData(theme_data_name);
    setAttribute("order", "-2");
}

void ThemeImage::updateRenderData()
{
    const ThemeData::StateData& t = theme->states[int(getState())];
    
    render_data.shader = Shader::get("internal:basic.shader");
    render_data.mesh = createStretched(getRenderSize());
    render_data.texture = t.texture;
    render_data.color = t.color;
}

};//namespace gui
};//namespace sp
