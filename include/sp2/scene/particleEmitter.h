#ifndef SP2_SCENE_PARTICLE_EMITTER_H
#define SP2_SCENE_PARTICLE_EMITTER_H

#include <sp2/scene/node.h>

namespace sp {

class ParticleEmitter : public Node
{
public:
    class Parameters
    {
    public:
        Vector3f position;
        Vector3f velocity;
        Vector3f acceleration;
        float start_size;
        float end_size;
        Vector3f start_color;
        Vector3f end_color; 
        float lifetime;
        float time;

        Parameters()
        : position(0, 0, 0), velocity(0, 0, 0), acceleration(0, 0, 0)
        , start_size(1.0), end_size(1.0)
        , start_color(1, 1, 1), end_color(0, 0, 0)
        , lifetime(1.0), time(0.0)
        {
        }
    };
    enum class Origin
    {
        Local,
        Global
    };

    /**
        Create a new particle emitter.
        Particle emitters are a efficient way to render a lot of camera facing sprites as a single group.
        Primary examples are explosions and smoke.
        
        You can specify how many particles you are expecting to use.
        This allocates a initial buffer, which increases performance if you know before hand how many particles you are going to create.
        For example, explosions have a well pre-defined amount of particles.
        Using more particles then the initial buffer is not an issue, the buffer will be re-sized.
        
        You can also have the option to create particles in local or global coordinates. Local particles move with this node, while global are created at this node location and not moved when this node it moved.
        Example:
        * Laser beam: local
        * Smoke: global
    */
    ParticleEmitter(P<Node> parent, int initial_buffer_size=16, Origin origin=Origin::Local);
    
    void emit(const Parameters& parameters);
    
    virtual void onUpdate(float delta) override;

    bool auto_destroy;

private:
    Origin origin;
    std::vector<Parameters> particles;
};

};//namespace sp

#endif//SP2_SCENE_PARTICLE_EMITTER_H
