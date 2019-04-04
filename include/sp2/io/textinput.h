#ifndef SP2_IO_TEXTINPUT_H
#define SP2_IO_TEXTINPUT_H

namespace sp {

enum class TextInputEvent
{
    Left,
    Right,
    Up,
    Down,
    LineStart,
    LineEnd,
    TextStart,
    TextEnd,
    Delete,
    Backspace,
    Return
};

};//namespace sp

#endif//SP2_IO_TEXTINPUT_H