#include <sp2/graphics/scene/graphicslayer.h>
#include <sp2/graphics/scene/renderpass.h>
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

void SceneGraphicsLayer::render(sf::RenderTarget& target)
{
    int pixel_width = viewport.size.x * target.getSize().x;
    int pixel_height = viewport.size.y * target.getSize().y;
    glViewport(viewport.position.x * target.getSize().x, viewport.position.y * target.getSize().y, pixel_width, pixel_height);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);
    
    float aspect_ratio = double(pixel_width) / double(pixel_height);

    for(RenderPass* pass : render_passes)
    {
        pass->render(this, aspect_ratio);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    sf::Shader::bind(nullptr);
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

void SceneGraphicsLayer::createRenderTarget(string name)
{
    targets[name] = new sf::RenderTexture();
}

};//namespace sp
