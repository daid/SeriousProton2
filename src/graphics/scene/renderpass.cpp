#include <sp2/graphics/scene/renderpass.h>

namespace sp {

RenderPass::RenderPass()
{
}

bool RenderPass::onPointerMove(Vector2d position, int id)
{
    return false;
}

void RenderPass::onPointerLeave(int id)
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

bool RenderPass::onWheelMove(Vector2d position, io::Pointer::Wheel direction)
{
    return false;
}

void RenderPass::onTextInput(const string& text)
{
}

void RenderPass::onTextInput(TextInputEvent e)
{
}

}//namespace sp
