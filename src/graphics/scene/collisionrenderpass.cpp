#include <sp2/graphics/scene/collisionrenderpass.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/opengl.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/camera.h>
#include <sp2/logging.h>
#include <sp2/io/keybinding.h>
#include <sp2/collision/2d/box2dBackend.h>
#include <sp2/collision/3d/bullet3dBackend.h>

#include <private/collision/box2d.h>
#include <private/collision/box2dVector.h>

#include <private/collision/bullet.h>
#include <private/collision/bulletVector.h>

#include <SDL_keyboard.h>


namespace sp {

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
    queue.add([]() { glDisable(GL_DEPTH_TEST); });
    if (specific_camera)
    {
        renderScene(queue, *specific_camera->getScene());
    }
    else
    {
        for(P<Scene> scene : Scene::all())
            renderScene(queue, scene);
    }
    queue.add([]() { glEnable(GL_DEPTH_TEST); });
}

void CollisionRenderPass::renderScene(RenderQueue& queue, P<Scene> scene)
{
    P<Camera> camera = scene->getCamera();
    if (specific_camera)
        camera = specific_camera;

    if (scene->isEnabled() && camera)
    {
        if (scene->collision_backend)
        {
            scene->collision_backend->getDebugRenderMesh(mesh);

            if (mesh)
            {
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
}

}//namespace sp
