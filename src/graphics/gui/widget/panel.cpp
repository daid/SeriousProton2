#include <sp2/graphics/gui/widget/panel.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

Panel::Panel(P<Widget> parent)
: Widget(parent)
{
    loadThemeStyle("panel");
}

void Panel::updateRenderData()
{
    updateRenderDataToThemeImage();
}

bool Panel::onPointerMove(Vector2d position, int id)
{
    return true;
}

bool Panel::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    return true;
}

}//namespace gui
}//namespace sp
