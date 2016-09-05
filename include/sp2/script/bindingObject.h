#ifndef SP2_SCRIPT_BINDING_OBJECT_H
#define SP2_SCRIPT_BINDING_OBJECT_H

#include <sp2/pointer.h>
#include <sp2/string.h>

namespace sp {

class ScriptBindingObject : public AutoPointerObject
{
public:
    void setScriptClass(string name);
};

};//!namespace sp

#endif//SP2_SCRIPT_BINDING_OBJECT_H
