#ifndef SP2_SCRIPT_BINDING_OBJECT_H
#define SP2_SCRIPT_BINDING_OBJECT_H

#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/script/bindingClass.h>

namespace sp {

class ScriptBindingObject : public AutoPointerObject
{
public:
    ScriptBindingObject();
    virtual ~ScriptBindingObject();

protected:
    virtual void onRegisterScriptBindings(ScriptBindingClass& script_binding_class);
    
    friend int script::lazyLoading(lua_State* L);
};

};//!namespace sp

#endif//SP2_SCRIPT_BINDING_OBJECT_H
