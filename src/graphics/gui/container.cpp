#include <sp2/graphics/gui/container.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

Container::Container()
{
}

Container::~Container()
{
    for(Widget* w : children)
    {
        delete w;
    }
}

};//!namespace gui
};//!namespace sp
