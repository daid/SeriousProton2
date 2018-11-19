#include <sp2/graphics/scene/graphicslayer.h>
#include <sp2/graphics/scene/renderpass.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <sp2/logging.h>
#include <sp2/graphics/opengl.h>
#include <sp2/graphics/shader.h>

namespace sp {

SceneGraphicsLayer::SceneGraphicsLayer(int priority)
: GraphicsLayer(priority)
{
}

SceneGraphicsLayer::~SceneGraphicsLayer()
{
    for(RenderPass* pass : render_passes)
        delete pass;
}

void SceneGraphicsLayer::render(RenderQueue& queue)
{
    queue.setAspectRatio(viewport.size.x / viewport.size.y);
    queue.add([]()
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_LEQUAL);
    });
    
    for(RenderPass* pass : render_passes)
        pass->render(queue);

    queue.add([]()
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        sp::Shader::unbind();
    });
}

bool SceneGraphicsLayer::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    position = screenToViewportPosition(position);
    if (position.x < -1.0 || position.y < -1.0 || position.x > 1.0 || position.y > 1.0)
        return false;
    for(RenderPass* pass : render_passes)
    {
        if (pass->onPointerDown(button, position, id))
        {
            pointer_render_pass[id] = pass;
            return true;
        }
    }
    return false;
}

void SceneGraphicsLayer::onPointerDrag(Vector2d position, int id)
{
    position = screenToViewportPosition(position);
    auto it = pointer_render_pass.find(id);
    if (it != pointer_render_pass.end() && it->second)
        it->second->onPointerDrag(position, id);
}

void SceneGraphicsLayer::onPointerUp(Vector2d position, int id)
{
    position = screenToViewportPosition(position);
    auto it = pointer_render_pass.find(id);
    if (it != pointer_render_pass.end() && it->second)
    {
        it->second->onPointerUp(position, id);
        pointer_render_pass.erase(it);
    }
}

void SceneGraphicsLayer::addRenderPass(P<RenderPass> render_pass)
{
    render_passes.add(render_pass);
}

};//namespace sp
