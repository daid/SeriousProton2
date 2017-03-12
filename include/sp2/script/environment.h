#ifndef SP2_SCRIPT_ENVIRONMENT_H
#define SP2_SCRIPT_ENVIRONMENT_H

#include <sp2/pointer.h>
#include <sp2/string.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/script/bindingObject.h>

namespace sp {
namespace script {

class Environment : public AutoPointerObject
{
public:
    Environment();
    virtual ~Environment();
    
    void setGlobal(string name, lua_CFunction function);
    void setGlobal(string name, P<ScriptBindingObject> ptr);
    
    bool load(sp::io::ResourceStreamPtr resource);
};

};//!namespace script
};//!namespace sp

#endif//SP2_SCRIPT_ENVIRONMENT_H
