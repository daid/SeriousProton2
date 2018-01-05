#include <sp2/graphics/scene/collisionrenderpass.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/cameraNode.h>
#include <sp2/logging.h>
#include <Box2D/Box2D.h>
#include <SFML/Window/Keyboard.hpp>
#include <private/collision/box2dVector.h>

namespace sp {

class Collision2DDebugRender : public b2Draw
{
public:
    Collision2DDebugRender()
    {
        SetFlags(e_shapeBit);
    }

	virtual void DrawPolygon(const b2Vec2* bvertices, int32 vertexCount, const b2Color& color) override
	{
        for(int n=0; n<vertexCount - 1; n+=2)
        {
            vertices.emplace_back(sf::Vector3f(bvertices[n].x, bvertices[n].y, 0.0f));
            vertices.emplace_back(sf::Vector3f(bvertices[(n + 1) % vertexCount].x, bvertices[(n + 1) % vertexCount].y, 0.0f));
            vertices.emplace_back(sf::Vector3f(bvertices[(n + 2) % vertexCount].x, bvertices[(n + 2) % vertexCount].y, 0.0f));
        }
	}

	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
	{
        DrawPolygon(vertices, vertexCount, color);
	}

	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override
	{
        for(int n=0; n<16; n++)
        {
            vertices.emplace_back(sf::Vector3f(center.x + std::sin(float(n + 1) / 8 * pi) * radius, center.y + std::cos(float(n + 1) / 8 * pi) * radius, 0.0f));
            vertices.emplace_back(sf::Vector3f(center.x + std::sin(float(n) / 8 * pi) * radius, center.y + std::cos(float(n) / 8 * pi) * radius, 0.0f));
            vertices.emplace_back(sf::Vector3f(center.x, center.y, 0.0f));
        }
	}
	
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override
	{
        DrawCircle(center, radius, color);
	}
	
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override
	{
        sp::Vector2f v0 = toVector<float>(p1);
        sp::Vector2f v1 = toVector<float>(p2);
        sp::Vector2f diff = sp::normalize(v1 - v0) * 0.2f;
        
        vertices.emplace_back(sf::Vector3f(v0.x, v0.y, 0.0f));
        vertices.emplace_back(sf::Vector3f(v1.x, v1.y, 0.0f));
        vertices.emplace_back(sf::Vector3f((v0.x + v1.x) / 2 - diff.y, (v0.y + v1.y) / 2 - diff.x, 0.0f));
        vertices.emplace_back(sf::Vector3f(v0.x, v0.y, 0.0f));
        vertices.emplace_back(sf::Vector3f(v1.x, v1.y, 0.0f));
        vertices.emplace_back(sf::Vector3f((v0.x + v1.x) / 2 + diff.y, (v0.y + v1.y) / 2 + diff.x, 0.0f));
	}

	virtual void DrawTransform(const b2Transform& xf) override
	{
        LOG(Debug, "Called unimplemented function", __FUNCTION__);
	}

	virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) override
	{
        size *= 0.05;
        vertices.emplace_back(sf::Vector3f(p.x - size, p.y - size, 0.0f));
        vertices.emplace_back(sf::Vector3f(p.x + size, p.y - size, 0.0f));
        vertices.emplace_back(sf::Vector3f(p.x - size, p.y + size, 0.0f));
        vertices.emplace_back(sf::Vector3f(p.x - size, p.y + size, 0.0f));
        vertices.emplace_back(sf::Vector3f(p.x + size, p.y - size, 0.0f));
        vertices.emplace_back(sf::Vector3f(p.x + size, p.y + size, 0.0f));
	}

    sp::MeshData::Vertices vertices;
};

CollisionRenderPass::CollisionRenderPass(string target_layer)
: RenderPass(target_layer)
{
    enabled = true;
    enabled_toggled = false;
}

CollisionRenderPass::CollisionRenderPass(string target_layer, P<Scene> scene)
: RenderPass(target_layer), single_scene(scene)
{
    enabled = true;
}

CollisionRenderPass::CollisionRenderPass(string target_layer, P<Scene> scene, P<CameraNode> camera)
: RenderPass(target_layer), single_scene(scene), specific_camera(camera)
{
    enabled = true;
}


void CollisionRenderPass::setScene(P<Scene> scene)
{
    single_scene = scene;
}

void CollisionRenderPass::setCamera(P<CameraNode> camera)
{
    specific_camera = camera;
}

void CollisionRenderPass::render(sf::RenderTarget& target, P<GraphicsLayer> layer, float aspect_ratio)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F2))
    {
        if (!enabled_toggled)
        {
            enabled_toggled = true;
            enabled = !enabled;
        }
    }
    else
    {
        enabled_toggled = false;
    }
    if (!enabled)
        return;
    if (single_scene)
    {
        renderScene(*single_scene, target, layer, aspect_ratio);
    }else{
        for(Scene* scene : Scene::scenes)
        {
            renderScene(scene, target, layer, aspect_ratio);
        }
    }
}

void CollisionRenderPass::renderScene(Scene* scene, sf::RenderTarget& target, P<GraphicsLayer> layer, float aspect_ratio)
{    
    P<CameraNode> camera = scene->getCamera();
    if (specific_camera && specific_camera->getScene() == scene)
        camera = specific_camera;
    
    if (scene->isEnabled() && camera)
    {
        camera->setAspectRatio(aspect_ratio);
        if (scene->collision_world2d)
        {
            Collision2DDebugRender debug_renderer;
            
            scene->collision_world2d->SetDebugDraw(&debug_renderer);
            scene->collision_world2d->DrawDebugData();
            scene->collision_world2d->SetDebugDraw(nullptr);
            
            if (!mesh)
                mesh = sp::MeshData::create(std::move(debug_renderer.vertices));
            else
                mesh->update(std::move(debug_renderer.vertices));

            queue.clear();
            RenderData render_data;
            render_data.shader = sp::Shader::get("internal:color.shader");
            render_data.type = sp::RenderData::Type::Normal;
            render_data.mesh = mesh;
            render_data.color = Color(255, 255, 255, 64);
            queue.add(Matrix4x4d::identity(), render_data);

            queue.render(camera->getProjectionMatrix(), camera->getGlobalTransform().inverse(), target);
        }
    }
}

};//!namespace sp
