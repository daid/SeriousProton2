#ifndef SP2_SCENE_PARTICLE_EMITTER_H
#define SP2_SCENE_PARTICLE_EMITTER_H

#include <sp2/scene/node.h>
#include <sp2/tween.h>


namespace sp {

class ParticleEmitter : public Node
{
public:
    class Parameters;

    class Effector : sp::NonCopyable
    {
    public:
        virtual ~Effector() = default;
        //Apply an effect to a particle. "f" is the lifetime of the particle in the range 0.0-1.0
        virtual void effect(Parameters& particle, float delta_time, float f) = 0;
    };
    template<typename T> class KeypointEffector : public Effector
    {
    public:
        KeypointEffector(const T& start_value, const T& end_value)
        {
            addKeypoint(0.0f, start_value);
            addKeypoint(1.0f, end_value);
        }

        KeypointEffector(std::initializer_list<std::pair<float, T>> keypoints)
        {
            for(auto& it : keypoints)
                addKeypoint(it.first, it.second);
        }

        KeypointEffector(const std::vector<std::pair<float, T>>& keypoints)
        {
            for(auto& it : keypoints)
                addKeypoint(it.first, it.second);
        }

        void addKeypoint(float f, const T& value)
        {
            keypoints.emplace_back(f, value);
            std::sort(keypoints.begin(), keypoints.end(), [](const std::pair<float, T>& a, const std::pair<float, T>& b) { return a.first < b.first;} );
        }
    protected:
        T getValue(float f)
        {
            if (f < keypoints.front().first)
                return keypoints.front().second;
            for(size_t idx=1; idx<keypoints.size(); idx++)
            {
                if (f < keypoints[idx].first)
                    return Tween<T>::linear(f, keypoints[idx-1].first, keypoints[idx].first, keypoints[idx-1].second, keypoints[idx].second);
            }
            return keypoints.back().second;
        }
    private:
        std::vector<std::pair<float, T>> keypoints;
    };
    class SizeEffector : public KeypointEffector<float>
    {
    public:
        using KeypointEffector<float>::KeypointEffector;

        virtual void effect(Parameters& particle, float delta_time, float f) override
        {
            particle.size = getValue(f);
        }
    };
    class ColorEffector : public KeypointEffector<Color>
    {
    public:
        using KeypointEffector<Color>::KeypointEffector;

        virtual void effect(Parameters& particle, float delta_time, float f) override
        {
            particle.color = getValue(f);
        }
    };
    class AlphaEffector : public KeypointEffector<float>
    {
    public:
        using KeypointEffector<float>::KeypointEffector;

        virtual void effect(Parameters& particle, float delta_time, float f) override
        {
            particle.color.a = getValue(f);
        }
    };
    class VelocityScaleEffector : public KeypointEffector<float>
    {
    public:
        using KeypointEffector<float>::KeypointEffector;

        virtual void effect(Parameters& particle, float delta_time, float f) override
        {
            particle.velocity *= 1.0f + getValue(f) * delta_time;
        }
    };
    class ConstantAcceleration : public Effector
    {
    public:
        ConstantAcceleration(sp::Vector3f acceleration)
        : acceleration(acceleration)
        {
        }

        virtual void effect(Parameters& particle, float delta_time, float f) override
        {
            particle.velocity += acceleration * delta_time;
        }
    private:
        sp::Vector3f acceleration;
    };

    class Parameters
    {
    public:
        Vector3f position;
        Vector3f velocity;
        float size;
        Color color;
        float lifetime;
        float time;

        Parameters()
        : position(0, 0, 0), velocity(0, 0, 0)
        , size(1.0)
        , color(1, 1, 1)
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

    template<typename T, typename... ARGS> void addEffector(ARGS... args)
    {
        effectors.emplace_back(std::unique_ptr<Effector>(new T(args...)));
    }

    void clearEffectors()
    {
        effectors.clear();
    }
    
    void emit(const Parameters& parameters);
    
    virtual void onUpdate(float delta) override;

    bool auto_destroy;

private:
    Origin origin;
    std::vector<Parameters> particles;
    std::vector<std::unique_ptr<Effector>> effectors;
};

}//namespace sp

#endif//SP2_SCENE_PARTICLE_EMITTER_H
