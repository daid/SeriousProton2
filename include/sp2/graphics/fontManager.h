#ifndef SP2_GRAPHICS_FONTMANAGER_H
#define SP2_GRAPHICS_FONTMANAGER_H

#include <sp2/io/directLoader.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/graphics/font.h>

namespace sp {

class FontManager : public io::DirectLoader<Font*>
{
protected:
    virtual Font* load(string name);
};

extern FontManager font_manager;

};//namespace sp

#endif//SP2_GRAPHICS_FONTMANAGER_H
