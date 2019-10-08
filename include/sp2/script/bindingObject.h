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

    void setScriptMember(string name, int value);
    void setScriptMember(string name, double value);
    void setScriptMember(string name, string value);
    int getScriptMemberInteger(string name);
    double getScriptMemberDouble(string name);
    string getScriptMemberString(string name);
protected:
    virtual void onRegisterScriptBindings(ScriptBindingClass& script_binding_class);
    
    friend void script::lazyLoading(int table_index, lua_State* L);
};

}//namespace sp

#endif//SP2_SCRIPT_BINDING_OBJECT_H
