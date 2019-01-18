#include <sp2/graphics/scene/renderpass.h>

namespace sp {

RenderPass::RenderPass()
{
}

bool RenderPass::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    return false;
}

void RenderPass::onPointerDrag(Vector2d position, int id)
{
}

void RenderPass::onPointerUp(Vector2d position, int id)
{
}

void RenderPass::onTextInput(const string& text)
{
}

void RenderPass::onTextInput(TextInputEvent e)
{
}

};//namespace sp
