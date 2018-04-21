#include <sp2/updatable.h>

namespace sp {

PList<Updatable> Updatable::updatables;

Updatable::Updatable()
{
    updatables.add(this);
}

};//!namespace sp
