#include <sp2/graphics/scene/collisionrenderpass.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/cameraNode.h>
#include <sp2/logging.h>
#include <Box2D/Box2D.h>
#include <SFML/Window/Keyboard.hpp>

namespace sp {

class Collision2DDebugRender : public b2Draw
{
public:
    Collision2DDebugRender(RenderQueue& queue)
    : queue(queue)
    {
        SetFlags(e_shapeBit);
    }

	virtual void DrawPolygon(const b2Vec2* bvertices, int32 vertexCount, const b2Color& color) override
	{
        RenderData data;

        std::vector<sp::MeshData::Vertex> vertices;
        for(int n=0; n<vertexCount - 1; n+=2)
        {
            vertices.emplace_back(sf::Vector3f(bvertices[n].x, bvertices[n].y, 0.0f));
            vertices.emplace_back(sf::Vector3f(bvertices[(n + 1) % vertexCount].x, bvertices[(n + 1) % vertexCount].y, 0.0f));
            vertices.emplace_back(sf::Vector3f(bvertices[(n + 2) % vertexCount].x, bvertices[(n + 2) % vertexCount].y, 0.0f));
        }
        
        data.shader = sp::Shader::get("shader/color.shader");
        data.type = sp::RenderData::Type::Normal;
        data.mesh = sp::MeshData::create(vertices);
        data.color = Color(color.r * 255, color.g * 255, color.b * 255, color.a * 64);

        queue.add(Matrix4x4d::identity(), data);
	}

	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
	{
        DrawPolygon(vertices, vertexCount, color);
	}

	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override
	{
        RenderData data;

        std::vector<sp::MeshData::Vertex> vertices;
        for(int n=0; n<16; n++)
        {
            vertices.emplace_back(sf::Vector3f(std::sin(float(n + 1) / 8 * pi) * radius, std::cos(float(n + 1) / 8 * pi) * radius, 0.0f));
            vertices.emplace_back(sf::Vector3f(std::sin(float(n) / 8 * pi) * radius, std::cos(float(n) / 8 * pi) * radius, 0.0f));
            vertices.emplace_back(sf::Vector3f(0.0f, 0.0f, 0.0f));
        }
        
        data.shader = sp::Shader::get("shader/color.shader");
        data.type = sp::RenderData::Type::Normal;
        data.mesh = sp::MeshData::create(vertices);
        data.color = Color(color.r * 255, color.g * 255, color.b * 255, color.a * 128);

        queue.add(Matrix4x4d::translate(center.x, center.y, 0), data);
	}
	
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override
	{
        DrawCircle(center, radius, color);
	}
	
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override
	{
        LOG(Debug, "Called unimplemented function", __FUNCTION__);
	}

	virtual void DrawTransform(const b2Transform& xf) override
	{
        LOG(Debug, "Called unimplemented function", __FUNCTION__);
	}

	virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) override
	{
        LOG(Debug, "Called unimplemented function", __FUNCTION__);
	}

    RenderQueue& queue;
};

CollisionRenderPass::CollisionRenderPass(string target_layer)
: RenderPass(target_layer)
{
    enabled = true;
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
        enabled = !enabled;
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
        Collision2DDebugRender debug_renderer(queue);
        
        camera->setAspectRatio(aspect_ratio);
        queue.clear();
        if (scene->collision_world2d)
        {
            scene->collision_world2d->SetDebugDraw(&debug_renderer);
            scene->collision_world2d->DrawDebugData();
            scene->collision_world2d->SetDebugDraw(nullptr);
        }
        queue.render(camera->getProjectionMatrix(), camera->getGlobalTransform().inverse(), target);
    }
}

};//!namespace sp
