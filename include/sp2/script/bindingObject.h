#ifndef SP2_SCRIPT_BINDING_OBJECT_H
#define SP2_SCRIPT_BINDING_OBJECT_H

#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/script/bindingClass.h>
#include <sp2/attributes.h>

namespace sp {
namespace script {

class BindingObject : public AutoPointerObject
{
public:
    BindingObject();
    virtual ~BindingObject();

protected:
    virtual void onRegisterScriptBindings(BindingClass& script_binding_class);

private:
    void registerToLua(lua_State* L);

    lua_State* lua = nullptr;
    friend void lazyLoading(int table_index, lua_State* L);
    friend int pushToLua(lua_State* L, BindingObject* ptr);
};

}//namespace script
}//namespace sp

#endif//SP2_SCRIPT_BINDING_OBJECT_H
