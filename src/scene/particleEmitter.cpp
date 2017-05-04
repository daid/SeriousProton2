#include <sp2/scene/particleEmitter.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/tween.h>

namespace sp {

ParticleEmitter::ParticleEmitter(P<SceneNode> parent, int initial_buffer_size, Origin origin)
: sp::SceneNode(parent)
{
    particles.reserve(initial_buffer_size);
    render_data.shader = Shader::get("shader/particle.shader");
    render_data.texture = "particle.png";
    render_data.type = RenderData::Type::Additive;
}

void ParticleEmitter::emit(const Parameters& parameters)
{
    particles.push_back(parameters);
}

void ParticleEmitter::onUpdate(float delta)
{
    std::vector<MeshData::Vertex> vertices;

    for(auto it = particles.begin(); it != particles.end(); )
    {
        Parameters& particle = *it;
        
        particle.time += delta;
        if (particle.time >= particle.lifetime)
        {
            if (it + 1 == particles.end())
            {
                it = particles.erase(it);
            }else
            {
                *it = particles.back();
                particles.erase(particles.end() - 1);
            }
        }else{
            particle.velocity += particle.acceleration * delta;
            particle.position += particle.velocity * delta;
            
            float size = Tween<float>::linear(particle.time, 0, particle.lifetime, particle.start_size, particle.end_size);
            sp::Vector3f color = Tween<sp::Vector3f>::linear(particle.time, 0, particle.lifetime, particle.start_color, particle.end_color);
            vertices.emplace_back(particle.position, sp::Vector3f(color.x, color.y, color.z), sp::Vector2f(-size,-size));
            vertices.emplace_back(particle.position, sp::Vector3f(color.x, color.y, color.z), sp::Vector2f( size,-size));
            vertices.emplace_back(particle.position, sp::Vector3f(color.x, color.y, color.z), sp::Vector2f(-size, size));

            vertices.emplace_back(particle.position, sp::Vector3f(color.x, color.y, color.z), sp::Vector2f(-size, size));
            vertices.emplace_back(particle.position, sp::Vector3f(color.x, color.y, color.z), sp::Vector2f( size,-size));
            vertices.emplace_back(particle.position, sp::Vector3f(color.x, color.y, color.z), sp::Vector2f( size, size));
            
            it++;
        }
    }
    
    render_data.mesh = MeshData::create(vertices);
}

};//!namespace sp
