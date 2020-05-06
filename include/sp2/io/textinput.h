#ifndef SP2_IO_TEXTINPUT_H
#define SP2_IO_TEXTINPUT_H

namespace sp {

enum class TextInputEvent
{
    Left,
    Right,
    WordLeft,
    WordRight,
    Up,
    Down,
    LineStart,
    LineEnd,
    TextStart,
    TextEnd,

    LeftWithSelection,
    RightWithSelection,
    WordLeftWithSelection,
    WordRightWithSelection,
    UpWithSelection,
    DownWithSelection,
    LineStartWithSelection,
    LineEndWithSelection,
    TextStartWithSelection,
    TextEndWithSelection,

    SelectAll,

    Delete,
    Backspace,
    Indent,
    Unindent,
    Return,

    Copy,
    Paste,
    Cut,
};

}//namespace sp

#endif//SP2_IO_TEXTINPUT_H
