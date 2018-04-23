#ifndef SP2_UPDATABLE_H
#define SP2_UPDATABLE_H

#include <sp2/pointerList.h>

namespace sp {

/**
    An updatable is an object that gets an update callback every tick and is not attached to any scene.
    General use of this is global object that needs to handle some global resource, like network communication.
 */
class Updatable : public AutoPointerObject
{
public:
    Updatable();

    virtual void onUpdate(float delta) = 0;

private:
    static PList<Updatable> updatables;
    
    friend class Engine;
};

};//namespace sp

#endif//SP2_UPDATABLE_H
