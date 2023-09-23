#include <sp2/scene/particleEmitter.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/io/keyValueTreeLoader.h>
#include <sp2/stringutil/convert.h>
#include <sp2/tween.h>
#include <sp2/random.h>

namespace sp {

static void parseParam(const string& s, float& f_min, float& f_max)
{
    auto p = s.partition("~");
    f_min = stringutil::convert::toFloat(p.first.strip());
    if (p.second.empty())
        f_max = f_min;
    else
        f_max = stringutil::convert::toFloat(p.second.strip());
}

static void parseParam(const string& s, Color& c_min, Color& c_max)
{
    auto p = s.partition("~");
    c_min = stringutil::convert::toColor(p.first.strip());
    if (p.second.empty())
        c_max = c_min;
    else
        c_max = stringutil::convert::toColor(p.second.strip());
}

static void parseParam(const string& s, Vector3f& f_min, Vector3f& f_max)
{
    auto p = s.split(",");
    parseParam(p[0], f_min.x, f_max.x);
    f_min.z = f_min.y = f_min.x;
    f_max.z = f_max.y = f_max.x;
    if (p.size() > 1)
        parseParam(p[1], f_min.y, f_max.y);
    if (p.size() > 2)
        parseParam(p[2], f_min.z, f_max.z);
}

ParticleEmitter::ParticleEmitter(P<Node> parent, string resource_name)
: sp::Node(parent), origin(Origin::Global)
{
    render_data.type = RenderData::Type::Transparent;

    auto tree = io::KeyValueTreeLoader::loadResource(resource_name);
    if (!tree)
        return;
    for(auto& root_node : tree->root_nodes)
    {
        if (root_node.items["texture"] != "")
            render_data.texture = texture_manager.get(root_node.items["texture"]);
        if (root_node.items["origin"].lower() == "local")
            origin = Origin::Local;
        if (root_node.items["acceleration"] != "")
            addEffector<ConstantAcceleration>(stringutil::convert::toVector3f(root_node.items["acceleration"]));
        if (root_node.items["renderType"] == "additive")
            render_data.type = RenderData::Type::Additive;
    }
    for(auto spawn_node : tree->findAllId("SPAWN"))
    {
        spawners.emplace_back();
        auto& spawner = spawners.back();
        float frequency = stringutil::convert::toFloat(spawn_node->items["frequency"]);
        if (frequency > 0)
            spawner.timer.repeat(1.0f / frequency);
        parseParam(spawn_node->items["position"], spawner.min.position, spawner.max.position);
        parseParam(spawn_node->items["velocity"], spawner.min.velocity, spawner.max.velocity);
        parseParam(spawn_node->items["size"], spawner.min.size, spawner.max.size);
        parseParam(spawn_node->items["color"], spawner.min.color, spawner.max.color);
        parseParam(spawn_node->items["lifetime"], spawner.min.lifetime, spawner.max.lifetime);

        int initial = stringutil::convert::toInt(spawn_node->items["initial"]);
        if (initial > 0)
        {
            for(int n=0; n<initial; n++)
            {
                Parameters p;
                p.position.x = random(spawner.min.position.x, spawner.max.position.x);
                p.position.y = random(spawner.min.position.y, spawner.max.position.y);
                p.position.z = random(spawner.min.position.z, spawner.max.position.z);
                p.velocity.x = random(spawner.min.velocity.x, spawner.max.velocity.x);
                p.velocity.y = random(spawner.min.velocity.y, spawner.max.velocity.y);
                p.velocity.z = random(spawner.min.velocity.z, spawner.max.velocity.z);
                p.size = random(spawner.min.size, spawner.max.size);
                p.color.r = random(spawner.min.color.r, spawner.max.color.r);
                p.color.g = random(spawner.min.color.g, spawner.max.color.g);
                p.color.b = random(spawner.min.color.b, spawner.max.color.b);
                p.color.a = random(spawner.min.color.a, spawner.max.color.a);
                p.lifetime = random(spawner.min.lifetime, spawner.max.lifetime);
                emit(p);
            }

            if (frequency <= 0.0)
                auto_destroy = true;
        }
    }
    auto size_node = tree->findId("SIZE");
    if (size_node)
    {
        std::vector<std::pair<float, float>> values;
        for(auto it : size_node->items)
            values.emplace_back(stringutil::convert::toFloat(it.first), stringutil::convert::toFloat(it.second));
        addEffector<SizeEffector>(values);
    }
    auto color_node = tree->findId("COLOR");
    if (color_node)
    {
        std::vector<std::pair<float, Color>> values;
        for(auto it : color_node->items)
            values.emplace_back(stringutil::convert::toFloat(it.first), stringutil::convert::toColor(it.second));
        addEffector<ColorEffector>(values);
    }
    auto alpha_node = tree->findId("ALPHA");
    if (alpha_node)
    {
        std::vector<std::pair<float, float>> values;
        for(auto it : alpha_node->items)
            values.emplace_back(stringutil::convert::toFloat(it.first), stringutil::convert::toFloat(it.second));
        addEffector<AlphaEffector>(values);
    }
    auto velocity_scale_node = tree->findId("VELOCITY_SCALE");
    if (velocity_scale_node)
    {
        std::vector<std::pair<float, float>> values;
        for(auto it : velocity_scale_node->items)
            values.emplace_back(stringutil::convert::toFloat(it.first), stringutil::convert::toFloat(it.second));
        addEffector<VelocityScaleEffector>(values);
    }

    switch(origin)
    {
    case Origin::Global:
        render_data.shader = Shader::get("internal:global_particle.shader");
        break;
    case Origin::Local:
        render_data.shader = Shader::get("internal:local_particle.shader");
        break;
    }
}

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
    render_data.texture = texture_manager.get("particle.png");
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
        p.velocity = getGlobalRotation3D() * p.velocity;
    }
}

void ParticleEmitter::onUpdate(float delta)
{
    for(auto& spawner : spawners) {
        while(spawner.timer.isExpired())
        {
            Parameters p;
            p.position.x = random(spawner.min.position.x, spawner.max.position.x);
            p.position.y = random(spawner.min.position.y, spawner.max.position.y);
            p.position.z = random(spawner.min.position.z, spawner.max.position.z);
            p.velocity.x = random(spawner.min.velocity.x, spawner.max.velocity.x);
            p.velocity.y = random(spawner.min.velocity.y, spawner.max.velocity.y);
            p.velocity.z = random(spawner.min.velocity.z, spawner.max.velocity.z);
            p.size = random(spawner.min.size, spawner.max.size);
            p.color.r = random(spawner.min.color.r, spawner.max.color.r);
            p.color.g = random(spawner.min.color.g, spawner.max.color.g);
            p.color.b = random(spawner.min.color.b, spawner.max.color.b);
            p.color.a = random(spawner.min.color.a, spawner.max.color.a);
            p.lifetime = random(spawner.min.lifetime, spawner.max.lifetime);
            emit(p);
        }
    }

    MeshData::Vertices vertices;
    MeshData::Indices indices;
    vertices.reserve(particles.size() * 4);
    indices.reserve(particles.size() * 6);

    for(auto it = particles.begin(); it != particles.end(); )
    {
        Parameters& particle = *it;

        float f = particle.time / particle.lifetime;
        for(auto& effector : effectors)
            effector->effect(particle, delta, f);

        particle.position += particle.velocity * delta;

        float size = particle.size;
        Color color = particle.color;

        indices.emplace_back(vertices.size() + 0);
        indices.emplace_back(vertices.size() + 1);
        indices.emplace_back(vertices.size() + 2);
        indices.emplace_back(vertices.size() + 2);
        indices.emplace_back(vertices.size() + 1);
        indices.emplace_back(vertices.size() + 3);

        vertices.emplace_back(particle.position, sp::Vector3f(color.r, color.g, color.b), sp::Vector2f(-size,-color.a));
        vertices.emplace_back(particle.position, sp::Vector3f(color.r, color.g, color.b), sp::Vector2f( size,-color.a));
        vertices.emplace_back(particle.position, sp::Vector3f(color.r, color.g, color.b), sp::Vector2f(-size, color.a));
        vertices.emplace_back(particle.position, sp::Vector3f(color.r, color.g, color.b), sp::Vector2f( size, color.a));

        particle.time += delta;
        if (particle.time >= particle.lifetime)
        {
            if (it + 1 == particles.end())
            {
                it = particles.erase(it);
            }
            else
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

void ParticleEmitter::startSpawn(float frequency)
{
    for(auto& spawner : spawners)
        spawner.timer.repeat(1.0f / frequency);
}

void ParticleEmitter::stopSpawn()
{
    for(auto& spawner : spawners)
        spawner.timer.stop();
}

}//namespace sp
