#include <sp2/graphics/scene/collisionrenderpass.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/camera.h>
#include <sp2/logging.h>
#include <sp2/io/keybinding.h>

#include <Box2D/Box2D.h>
#include <private/collision/box2dVector.h>

#include <btBulletDynamicsCommon.h>
#include <private/collision/bulletVector.h>

#include <SDL2/SDL_keyboard.h>


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
        Vector3f c(color.r, color.g, color.b);
        for(int n=0; n<vertexCount - 1; n+=2)
        {
            indices.emplace_back(vertices.size());
            vertices.emplace_back(Vector3f(bvertices[n].x, bvertices[n].y, 0.0f), c, Vector2f());
            indices.emplace_back(vertices.size());
            vertices.emplace_back(Vector3f(bvertices[(n + 1) % vertexCount].x, bvertices[(n + 1) % vertexCount].y, 0.0f), c, Vector2f());
            indices.emplace_back(vertices.size());
            vertices.emplace_back(Vector3f(bvertices[(n + 2) % vertexCount].x, bvertices[(n + 2) % vertexCount].y, 0.0f), c, Vector2f());
        }
	}

	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
	{
        DrawPolygon(vertices, vertexCount, color);
	}

	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override
	{
        Vector3f c(color.r, color.g, color.b);

        int index = vertices.size();
        vertices.emplace_back(Vector3f(center.x, center.y, 0.0f), c, Vector2f());
        for(int n=0; n<16; n++)
            vertices.emplace_back(Vector3f(center.x + std::sin(float(n) / 8 * pi) * radius, center.y + std::cos(float(n) / 8 * pi) * radius, 0.0f), c, Vector2f());
        for(int n=0; n<16; n++)
        {
            indices.emplace_back(index);
            indices.emplace_back(index + 1 + ((n + 1) % 16));
            indices.emplace_back(index + 1 + n);
        }
	}
	
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override
	{
        DrawCircle(center, radius, color);
	}
	
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override
	{
        Vector3f c(color.r, color.g, color.b);

        Vector2f v0 = toVector<float>(p1);
        Vector2f v1 = toVector<float>(p2);
        Vector2f diff = (v1 - v0).normalized() * 0.2f;
        
        int index = vertices.size();
        vertices.emplace_back(Vector3f(v0.x, v0.y, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(v1.x, v1.y, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f((v0.x + v1.x) / 2 - diff.y, (v0.y + v1.y) / 2 - diff.x, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f((v0.x + v1.x) / 2 + diff.y, (v0.y + v1.y) / 2 + diff.x, 0.0f), c, Vector2f());

        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 2);
        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 3);
	}

	virtual void DrawTransform(const b2Transform& xf) override
	{
        LOG(Debug, "Called unimplemented function", __FUNCTION__);
	}

	virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) override
	{
        size *= 0.05;
        int index = vertices.size();
        Vector3f c(color.r, color.g, color.b);
        vertices.emplace_back(Vector3f(p.x - size, p.y - size, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(p.x + size, p.y - size, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(p.x - size, p.y + size, 0.0f), c, Vector2f());
        vertices.emplace_back(Vector3f(p.x + size, p.y + size, 0.0f), c, Vector2f());

        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 2);
        indices.emplace_back(index + 2);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 3);
	}

    sp::MeshData::Vertices vertices;
    sp::MeshData::Indices indices;
};

class Collision3DDebugRender : public btIDebugDraw
{
public:
    virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
    {
        Vector3f c = toVector<float>(color);

        Vector3f v0 = toVector<float>(from);
        Vector3f v1 = toVector<float>(to);
        Vector3f diff = (v1 - v0);
        
        //This isn't perfect, but it works good enough.
        diff = diff.cross(sp::Vector3f(0, 0, 1)).normalized() * 0.2f;
        
        int index = vertices.size();
        vertices.emplace_back(v0, c, Vector2f());
        vertices.emplace_back(v1, c, Vector2f());
        vertices.emplace_back((v0 + v1) * 0.5f - diff, c, Vector2f());
        vertices.emplace_back((v0 + v1) * 0.5f + diff, c, Vector2f());

        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 2);
        indices.emplace_back(index);
        indices.emplace_back(index + 1);
        indices.emplace_back(index + 3);
    }

    virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
    {
    }

	virtual void reportErrorWarning(const char* warningString)
	{
	}

	virtual void draw3dText(const btVector3& location,const char* textString)
	{
	}
	
	virtual void setDebugMode(int debugMode)
	{
	}
	
	virtual int getDebugMode() const
	{
        return DBG_DrawWireframe;
	}

    sp::MeshData::Vertices vertices;
    sp::MeshData::Indices indices;
};

CollisionRenderPass::CollisionRenderPass()
{
    enabled = true;
    enabled_toggled = false;
}

CollisionRenderPass::CollisionRenderPass(P<Camera> camera)
: specific_camera(camera)
{
    enabled = true;
    enabled_toggled = false;
}

void CollisionRenderPass::setCamera(P<Camera> camera)
{
    specific_camera = camera;
}

void CollisionRenderPass::render(RenderQueue& queue)
{
    if (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_F2])
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
    if (specific_camera)
    {
        renderScene(queue, *specific_camera->getScene());
    }
    else
    {
        for(Scene* scene : Scene::scenes)
            renderScene(queue, scene);
    }
}

void CollisionRenderPass::renderScene(RenderQueue& queue, Scene* scene)
{    
    P<Camera> camera = scene->getCamera();
    if (specific_camera)
        camera = specific_camera;
    
    if (scene->isEnabled() && camera)
    {
        if (scene->collision_world2d)
        {
            Collision2DDebugRender debug_renderer;
            
            scene->collision_world2d->SetDebugDraw(&debug_renderer);
            scene->collision_world2d->DrawDebugData();
            scene->collision_world2d->SetDebugDraw(nullptr);
            
            if (!mesh)
                mesh = MeshData::create(std::move(debug_renderer.vertices), std::move(debug_renderer.indices), MeshData::Type::Dynamic);
            else
                mesh->update(std::move(debug_renderer.vertices), std::move(debug_renderer.indices));

            queue.setCamera(camera);
            RenderData render_data;
            render_data.shader = Shader::get("internal:normal_as_color.shader");
            render_data.type = RenderData::Type::Normal;
            render_data.mesh = mesh;
            render_data.color = Color(1, 1, 1, 0.25);
            queue.add(Matrix4x4f::identity(), render_data);
        }

        if (scene->collision_world3d)
        {
            Collision3DDebugRender debug_renderer;
            
            scene->collision_world3d->setDebugDrawer(&debug_renderer);
            scene->collision_world3d->debugDrawWorld();
            scene->collision_world3d->setDebugDrawer(nullptr);

            if (!mesh)
                mesh = MeshData::create(std::move(debug_renderer.vertices), std::move(debug_renderer.indices), MeshData::Type::Dynamic);
            else
                mesh->update(std::move(debug_renderer.vertices), std::move(debug_renderer.indices));

            queue.setCamera(camera);
            RenderData render_data;
            render_data.shader = Shader::get("internal:normal_as_color.shader");
            render_data.type = RenderData::Type::Normal;
            render_data.mesh = mesh;
            render_data.color = Color(1, 1, 1, 0.25);
            queue.add(Matrix4x4f::identity(), render_data);
        }
    }
}

};//namespace sp
