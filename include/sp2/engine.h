#ifndef SP2_ENGINE_H
#define SP2_ENGINE_H

#include <sp2/pointer.h>
#include <sp2/string.h>

namespace sp {

class Engine : public AutoPointerObject
{
public:
    static constexpr float fixed_update_frequency = 60.0f;
    static constexpr float fixed_update_delta = 1.0f / fixed_update_frequency;
    
    class UpdateTiming
    {
    public:
        float fixed_update;
        float dynamic_update;
        float render;
    } last_update_timing;

    Engine();
    virtual ~Engine();

    void run();

    void initialize();
    void processEvents();
    void update(float time_delta);
    
    void setGameSpeed(float speed);
    float getGameSpeed();
    void setPause(bool pause);
    bool getPause();
    void shutdown();
    
    // Check if we are currently calling onUpdateFixed.
    bool isInFixedUpdate() { return in_fixed_update; }
    
    float getFPS() { return current_fps; }
    
    static Engine* getInstance() { return *engine; }
private:
    static P<Engine> engine;

    float game_speed;
    bool paused;

    bool in_fixed_update;

    float maximum_frame_time;
    float minimum_frame_time;
    float fixed_update_accumulator;

    float current_fps = 0.0;
    bool initialized = false;
    bool shutdown_flag = false;
};

}//namespace sp

#endif//SP2_ENGINE_H

