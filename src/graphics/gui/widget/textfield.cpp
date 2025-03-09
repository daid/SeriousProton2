#include <sp2/graphics/gui/widget/textfield.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/stringutil/convert.h>
#include <sp2/engine.h>

namespace sp {
namespace gui {

TextField::TextField(P<Widget> parent)
: TextArea(parent)
{
    setAttribute("multiline", "false");
    loadThemeStyle("textfield");
}

}//namespace gui
}//namespace sp
