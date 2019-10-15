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

    void setScriptMember(const string& name, int value);
    void setScriptMember(const string& name, double value);
    void setScriptMember(const string& name, const string& value);
    int getScriptMemberInteger(const string& name);
    double getScriptMemberDouble(const string& name);
    string getScriptMemberString(const string& name);
protected:
    virtual void onRegisterScriptBindings(ScriptBindingClass& script_binding_class);
    
    friend void script::lazyLoading(int table_index, lua_State* L);
};

}//namespace sp

#endif//SP2_SCRIPT_BINDING_OBJECT_H
