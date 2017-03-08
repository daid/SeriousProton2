#include <sp2/scene/cameraNode.h>
#include <sp2/logging.h>

namespace sp {

CameraNode::CameraNode(P<SceneNode> parent)
: SceneNode(parent), type(Type::Ortographic)
{
    field_of_view = 60.0;
    view_distance = 1.0;
}

void CameraNode::setOrtographic(double view_distance)
{
    this->type = Type::Ortographic;
    this->view_distance = view_distance;
}

void CameraNode::setPerspective(double field_of_view, double view_distance)
{
    this->type = Type::Perspective;
    this->field_of_view = field_of_view;
    this->view_distance = view_distance;
}

void CameraNode::setAspectRatio(double ratio)
{
    switch(type)
    {
    case Type::Ortographic:
        setProjectionMatrix(Matrix4x4d::ortho(-ratio * view_distance, ratio * view_distance, -view_distance, view_distance, -view_distance, view_distance));
        break;
    case Type::Perspective:
        setProjectionMatrix(Matrix4x4d::perspective(field_of_view, ratio, 1.0f, view_distance));
        break;
    }
}

sp::Vector2d CameraNode::screenToWorld(sp::Vector2f position)
{
    //First transform the screen pixel coordinates into -1 to 1 coordinates to match OpenGL screen space.
    sp::Vector2d screen_position_normalized = sp::Vector2d(position) * 2.0 - sp::Vector2d(1, 1);
    screen_position_normalized.y = -screen_position_normalized.y;
    //Then apply our inversed projection matrix to go from screen coordinates to world coordinates relative to the camera
    sp::Vector2d world_position = projection_matrix.inverse() * screen_position_normalized;
    //Finally apply our own matrix to transform this into global space.
    return getLocalPoint2D(world_position);
}

sp::Vector2f CameraNode::worldToScreen(sp::Vector2d position)
{
    sp::Vector2d world_position = getLocalTransform().inverse() * position;
    sp::Vector2d screen_position_normalized = projection_matrix * world_position;
    screen_position_normalized.y = -screen_position_normalized.y;
    return (sp::Vector2f(screen_position_normalized) + sp::Vector2f(1, 1)) / 2.0f;
}

};//!namespace sp
