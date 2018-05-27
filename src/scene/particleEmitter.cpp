#include <sp2/scene/particleEmitter.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/tween.h>

namespace sp {

ParticleEmitter::ParticleEmitter(P<Node> parent, int initial_buffer_size, Origin origin)
: sp::Node(parent), origin(origin)
{
    particles.reserve(initial_buffer_size);
    switch(origin)
    {
    case Origin::Global:
        render_data.shader = Shader::get("internal:global_particle.shader");
        break;
    case Origin::Local:
        render_data.shader = Shader::get("internal:local_particle.shader");
        break;
    }
    render_data.texture = textureManager.get("particle.png");
    render_data.type = RenderData::Type::Transparent;
    
    auto_destroy = false;
}

void ParticleEmitter::emit(const Parameters& parameters)
{
    particles.push_back(parameters);
    if (origin == Origin::Global)
    {
        Parameters& p = particles.back();
        p.position += sp::Vector3f(getGlobalPosition3D());
    }
}

void ParticleEmitter::onUpdate(float delta)
{
    MeshData::Vertices vertices;
    MeshData::Indices indices;
    vertices.reserve(particles.size() * 4);
    indices.reserve(particles.size() * 6);

    for(auto it = particles.begin(); it != particles.end(); )
    {
        Parameters& particle = *it;
        
        particle.velocity += particle.acceleration * delta;
        particle.position += particle.velocity * delta;
        
        float size = Tween<float>::linear(particle.time, 0, particle.lifetime, particle.start_size, particle.end_size);
        Vector3f color = Tween<Vector3f>::linear(particle.time, 0, particle.lifetime, particle.start_color, particle.end_color);
        
        indices.emplace_back(vertices.size() + 0);
        indices.emplace_back(vertices.size() + 1);
        indices.emplace_back(vertices.size() + 2);
        indices.emplace_back(vertices.size() + 2);
        indices.emplace_back(vertices.size() + 1);
        indices.emplace_back(vertices.size() + 3);
        
        vertices.emplace_back(particle.position, sp::Vector3f(color.x, color.y, color.z), sp::Vector2f(-size,-size));
        vertices.emplace_back(particle.position, sp::Vector3f(color.x, color.y, color.z), sp::Vector2f( size,-size));
        vertices.emplace_back(particle.position, sp::Vector3f(color.x, color.y, color.z), sp::Vector2f(-size, size));
        vertices.emplace_back(particle.position, sp::Vector3f(color.x, color.y, color.z), sp::Vector2f( size, size));
        
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
        }
        else
        {
            it++;
        }
    }

    if (auto_destroy && vertices.size() == 0)
    {
        delete this;
        return;
    }
    
    if (!render_data.mesh)
        render_data.mesh = MeshData::create(std::move(vertices), std::move(indices), MeshData::Type::Dynamic);
    else
        render_data.mesh->update(std::move(vertices), std::move(indices));
}

};//namespace sp
