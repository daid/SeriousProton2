#ifndef SP2_ENGINE_H
#define SP2_ENGINE_H

#include <SFML/Graphics/RenderWindow.hpp>
#include <sp2/pointer.h>
#include <sp2/string.h>

namespace sp {

class Engine : public AutoPointerObject
{
public:
    static constexpr float fixed_update_frequency = 30.0f;

    Engine();
    virtual ~Engine();

    void run();
    
    void setGameSpeed(float speed);
    float getGameSpeed();
    void setPause(bool pause);
    bool getPause();
    void shutdown();
    
    static Engine* getInstance() { return *engine; }
private:
    static P<Engine> engine;

    float game_speed;
    bool paused;

    float maximum_frame_time;
    float minimum_frame_time;
    float fixed_update_accumulator;
};

};//!namespace sp

#endif//SP2_ENGINE_H

