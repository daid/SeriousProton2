#include <sp2/graphics/scene/graphicslayer.h>
#include <sp2/graphics/scene/renderpass.h>

namespace sp {

SceneGraphicsLayer::SceneGraphicsLayer(int priority)
: GraphicsLayer(priority)
{
}

void SceneGraphicsLayer::render(sf::RenderTarget& window)
{
    //TODO: Figure out proper rendering order.
    for(RenderPass* pass : render_passes)
    {
        string target = pass->getTargetLayer();
        if (target == "window")
            pass->render(window, this);
        else
            pass->render(*targets[target], this);
    }
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
}

void SceneGraphicsLayer::createRenderTarget(string name)
{
    targets[name] = new sf::RenderTexture();
}

};//!namespace sp
