#ifndef SP2_GRAPHICS_GUI_LAYOUT_H
#define SP2_GRAPHICS_GUI_LAYOUT_H

#include <sp2/pointerVector.h>
#include <sp2/graphics/gui/container.h>
#include <SFML/Graphics/Rect.hpp>

namespace sp {
namespace gui {

class Layout : sf::NonCopyable
{
public:
    virtual void update(P<Container> container, sf::FloatRect rect);
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_LAYOUT_H
