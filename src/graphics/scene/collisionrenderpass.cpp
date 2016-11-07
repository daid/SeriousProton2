#include <sp2/graphics/scene/collisionrenderpass.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/cameraNode.h>
#include <sp2/logging.h>
#include <Box2D/Box2D.h>

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
        data.mesh = std::make_shared<sp::MeshData>(vertices);
        data.color = sf::Color(color.r * 255, color.g * 255, color.b * 255, color.a * 128);

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
        data.mesh = std::make_shared<sp::MeshData>(vertices);
        data.color = sf::Color(color.r * 255, color.g * 255, color.b * 255, color.a * 128);

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
}
    
void CollisionRenderPass::render(sf::RenderTarget& target, P<GraphicsLayer> layer)
{
    for(Scene* scene : Scene::scenes)
    {
        P<CameraNode> camera = scene->getCamera();
        if (scene->isEnabled() && camera)
        {
            Collision2DDebugRender debug_renderer(queue);
            
            //TODO: Account for viewport, currently assumes viewport is on the whole target.
            camera->setAspectRatio(double(target.getSize().x) / double(target.getSize().y));
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
}

};//!namespace sp
