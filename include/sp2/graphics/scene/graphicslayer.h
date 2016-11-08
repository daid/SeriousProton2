#ifndef SP2_GRAPHICS_SCENE_GRAPHICSLAYER_H
#define SP2_GRAPHICS_SCENE_GRAPHICSLAYER_H

#include <sp2/graphics/graphicslayer.h>
#include <SFML/Graphics/RenderTexture.hpp>
#include <map>

namespace sp {

class RenderPass;
class SceneGraphicsLayer : public GraphicsLayer
{
public:
    SceneGraphicsLayer(int priority);
    virtual ~SceneGraphicsLayer();
    
    virtual void render(sf::RenderTarget& window) override;
    virtual bool onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id) override;
    virtual void onPointerDrag(sf::Vector2f position, int id) override;
    virtual void onPointerUp(sf::Vector2f position, int id) override;
    
    void addRenderPass(P<RenderPass> render_pass);
    void createRenderTarget(string name);
private:
    PVector<RenderPass> render_passes;
    std::map<string, sf::RenderTexture*> targets;
};

};//!namespace sp

#endif//SP2_GRAPHICS_SCENE_GRAPHICSLAYER_H
