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

void SceneGraphicsLayer::render(sf::RenderTarget& window)
{
    static GLuint vertex_array_id = 0;
    if (!vertex_array_id)
    {
        glGenVertexArrays(1, &vertex_array_id);
    }
    glViewport(0, 0, window.getSize().x, window.getSize().y);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vertex_array_id);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //TODO: Figure out proper rendering order.
    for(RenderPass* pass : render_passes)
    {
        string target = pass->getTargetLayer();
        if (target == "window")
            pass->render(window, this);
        else
            pass->render(*targets[target], this);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(0);
    sf::Shader::bind(nullptr);
}

bool SceneGraphicsLayer::onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id)
{
    return false;
}

void SceneGraphicsLayer::onPointerDrag(sf::Vector2f position, int id)
{
}

void SceneGraphicsLayer::onPointerUp(sf::Vector2f position, int id)
{
}

void SceneGraphicsLayer::addRenderPass(P<RenderPass> render_pass)
{
    render_passes.add(render_pass);
}

void SceneGraphicsLayer::createRenderTarget(string name)
{
    targets[name] = new sf::RenderTexture();
}

};//!namespace sp
