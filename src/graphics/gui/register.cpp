#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/widget/image.h>
#include <sp2/graphics/gui/widget/keynavigator.h>
#include <sp2/graphics/gui/widget/label.h>
#include <sp2/graphics/gui/widget/listbox.h>
#include <sp2/graphics/gui/widget/panel.h>
#include <sp2/graphics/gui/widget/progressbar.h>
#include <sp2/graphics/gui/widget/slider.h>
#include <sp2/graphics/gui/widget/textarea.h>
#include <sp2/graphics/gui/widget/textfield.h>
#include <sp2/graphics/gui/widget/togglebutton.h>
#include <sp2/graphics/gui/widget/tumbler.h>

#include <sp2/graphics/gui/layout/grid.h>
#include <sp2/graphics/gui/layout/horizontal.h>
#include <sp2/graphics/gui/layout/horizontalflow.h>
#include <sp2/graphics/gui/layout/vertical.h>
#include <sp2/graphics/gui/layout/verticalflow.h>

namespace sp {
namespace gui {
    
void registerDefaultWidgetsAndLayouts()
{
    SP_REGISTER_WIDGET("", Widget);
    SP_REGISTER_WIDGET("button", Button);
    SP_REGISTER_WIDGET("image", Image);
    SP_REGISTER_WIDGET("keynavigator", KeyNavigator);
    SP_REGISTER_WIDGET("label", Label);
    SP_REGISTER_WIDGET("listbox", Listbox);
    SP_REGISTER_WIDGET("panel", Panel);
    SP_REGISTER_WIDGET("progressbar", Progressbar);
    SP_REGISTER_WIDGET("slider", Slider);
    SP_REGISTER_WIDGET("textarea", TextArea);
    SP_REGISTER_WIDGET("textfield", TextField);
    SP_REGISTER_WIDGET("togglebutton", ToggleButton);
    SP_REGISTER_WIDGET("tumbler", Tumbler);

    SP_REGISTER_LAYOUT("default", Layout);
    SP_REGISTER_LAYOUT("grid", GridLayout);
    SP_REGISTER_LAYOUT("horizontal", HorizontalLayout);
    SP_REGISTER_LAYOUT("horizontalflow", HorizontalFlowLayout);
    SP_REGISTER_LAYOUT("vertical", VerticalLayout);
    SP_REGISTER_LAYOUT("verticalflow", VerticalFlowLayout);
}

}
}