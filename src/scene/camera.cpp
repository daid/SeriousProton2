#include <sp2/scene/camera.h>
#include <sp2/logging.h>

namespace sp {

Camera::Camera(P<Node> parent)
: Node(parent), type(Type::OrtographicFixedHeight)
{
    field_of_view = 60.0;
    view_distance = 1.0;
}

void Camera::setOrtographic(double view_distance, bool fixed_width)
{
    if (fixed_width)
        this->type = Type::OrtographicFixedWidth;
    else
        this->type = Type::OrtographicFixedHeight;
    this->view_distance = view_distance;
}

void Camera::setPerspective(double field_of_view, double view_distance)
{
    this->type = Type::Perspective;
    this->field_of_view = field_of_view;
    this->view_distance = view_distance;
}

void Camera::setAspectRatio(double ratio)
{
    switch(type)
    {
    case Type::OrtographicFixedHeight:
        setProjectionMatrix(Matrix4x4d::ortho(-ratio * view_distance, ratio * view_distance, -view_distance, view_distance, -view_distance, view_distance));
        break;
    case Type::OrtographicFixedWidth:
        setProjectionMatrix(Matrix4x4d::ortho(-view_distance, view_distance, -view_distance / ratio, view_distance / ratio, -view_distance, view_distance));
        break;
    case Type::Perspective:
        setProjectionMatrix(Matrix4x4d::perspective(field_of_view, ratio, 1.0f, view_distance));
        break;
    }
}

sp::Vector2d Camera::screenToWorld(sp::Vector2f position)
{
    //First transform the screen pixel coordinates into -1 to 1 coordinates to match OpenGL screen space.
    sp::Vector2d screen_position_normalized = sp::Vector2d(position) * 2.0 - sp::Vector2d(1, 1);
    screen_position_normalized.y = -screen_position_normalized.y;
    //Then apply our inversed projection matrix to go from screen coordinates to world coordinates relative to the camera
    sp::Vector2d world_position = projection_matrix.inverse() * screen_position_normalized;
    //Finally apply our own matrix to transform this into global space.
    return getLocalPoint2D(world_position);
}

sp::Vector2f Camera::worldToScreen(sp::Vector2d position)
{
    sp::Vector2d world_position = getLocalTransform().inverse() * position;
    sp::Vector2d screen_position_normalized = projection_matrix * world_position;
    screen_position_normalized.y = -screen_position_normalized.y;
    return (sp::Vector2f(screen_position_normalized) + sp::Vector2f(1, 1)) / 2.0f;
}

};//!namespace sp
