#ifndef SP2_GRAPHICS_GUI_CONTAINER_H
#define SP2_GRAPHICS_GUI_CONTAINER_H

#include <sp2/pointerVector.h>
#include <SFML/Graphics/Rect.hpp>

namespace sp {
namespace gui {

class Widget;
class Layout;
class Container : public AutoPointerObject
{
protected:
    PVector<Widget> children;
public:
    Container();
    virtual ~Container();

    friend class Widget;
    friend class GraphicsLayer;
};


};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_CONTAINER_H
