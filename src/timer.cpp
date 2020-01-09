#include <sp2/timer.h>
#include <sp2/engine.h>

namespace sp {

EngineTime::time_point EngineTime::now()
{
    return EngineTime::time_point(sp::Engine::getInstance()->getElapsedGameTime());
}

} //!namespace
