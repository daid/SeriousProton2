#include <sp2/scene/camera.h>
#include <sp2/logging.h>

namespace sp {

Camera::Camera(P<Node> parent)
: Node(parent), type(Type::OrtographicVertical)
{
    field_of_view = 60.0;
    view_distance = 1.0;
}

void Camera::setOrtographic(double view_distance, Direction direction)
{
    switch(direction)
    {
    case Direction::Horizontal: this->type = Type::OrtographicHorizontal; break;
    case Direction::Vertical: this->type = Type::OrtographicVertical; break;
    }
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
    case Type::OrtographicVertical:
        setProjectionMatrix(Matrix4x4f::ortho(-ratio * view_distance, ratio * view_distance, -view_distance, view_distance, -view_distance, view_distance));
        break;
    case Type::OrtographicHorizontal:
        setProjectionMatrix(Matrix4x4f::ortho(-view_distance, view_distance, -view_distance / ratio, view_distance / ratio, -view_distance, view_distance));
        break;
    case Type::Perspective:
        setProjectionMatrix(Matrix4x4f::perspective(field_of_view, ratio, 1.0f, view_distance));
        break;
    }
}

Vector2d Camera::screenToWorld(Vector2f position)
{
    //First transform the screen pixel coordinates into -1 to 1 coordinates to match OpenGL screen space.
    Vector2f screen_position_normalized = position * 2.0f - Vector2f(1, 1);
    screen_position_normalized.y = -screen_position_normalized.y;
    //Then apply our inversed projection matrix to go from screen coordinates to world coordinates relative to the camera
    Vector2f world_position = projection_matrix.inverse() * screen_position_normalized;
    //Finally apply our own matrix to transform this into global space.
    return getLocalPoint2D(Vector2d(world_position));
}

Vector2f Camera::worldToScreen(Vector2d position)
{
    Vector2f world_position = getLocalTransform().inverse() * Vector2f(position);
    Vector2f screen_position_normalized = projection_matrix * world_position;
    screen_position_normalized.y = -screen_position_normalized.y;
    return (screen_position_normalized + Vector2f(1, 1)) / 2.0f;
}

};//namespace sp
