#ifndef SP2_GRAPHICS_GUI_CONTAINER_H
#define SP2_GRAPHICS_GUI_CONTAINER_H

#include <sp2/pointerList.h>
#include <SFML/Graphics/Rect.hpp>

namespace sp {
namespace gui {

class Widget;
class Layout;
class Container : public AutoPointerObject
{
public:
    PList<Widget> children;

    Container();
    virtual ~Container();

    friend class Widget;
    friend class GraphicsLayer;
    friend class Layout;
};


};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_CONTAINER_H
