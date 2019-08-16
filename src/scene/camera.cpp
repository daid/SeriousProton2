#include <sp2/scene/camera.h>
#include <sp2/logging.h>

namespace sp {

Camera::Camera(P<Node> parent)
: Node(parent), type(Type::OrtographicVertical)
{
    field_of_view = 60.0;
    view_distance = 1.0;
}

void Camera::setOrtographic(double view_size, Direction direction, double view_distance)
{
    switch(direction)
    {
    case Direction::Horizontal: this->type = Type::OrtographicHorizontal; break;
    case Direction::Vertical: this->type = Type::OrtographicVertical; break;
    }
    this->field_of_view = view_size;
    this->view_distance = view_distance;
    if (this->view_distance < view_size)
        this->view_distance = view_size;
}

void Camera::setPerspective(double field_of_view, double view_distance)
{
    this->type = Type::PerspectiveVertical;
    this->field_of_view = field_of_view;
    this->view_distance = view_distance;
}

void Camera::setPerspective(double field_of_view, Direction direction, double view_distance)
{
    switch(direction)
    {
    case Direction::Horizontal: this->type = Type::PerspectiveHorizontal; break;
    case Direction::Vertical: this->type = Type::PerspectiveVertical; break;
    }
    this->field_of_view = field_of_view;
    this->view_distance = view_distance;
}

void Camera::setAspectRatio(double ratio)
{
    switch(type)
    {
    case Type::OrtographicVertical:
        setProjectionMatrix(Matrix4x4f::ortho(-ratio * field_of_view, ratio * field_of_view, -field_of_view, field_of_view, -view_distance, view_distance));
        break;
    case Type::OrtographicHorizontal:
        setProjectionMatrix(Matrix4x4f::ortho(-field_of_view, field_of_view, -field_of_view / ratio, field_of_view / ratio, -view_distance, view_distance));
        break;
    case Type::PerspectiveVertical:
        setProjectionMatrix(Matrix4x4f::perspective(field_of_view, ratio, 1.0f, view_distance));
        break;
    case Type::PerspectiveHorizontal:
        setProjectionMatrix(Matrix4x4f::perspectiveH(field_of_view, ratio, 1.0f, view_distance));
        break;
    }
}

Vector2f Camera::worldToScreen(Vector2d position)
{
    Vector2f world_position = getGlobalTransform().inverse() * Vector2f(position);
    return projection_matrix * world_position;
}

Vector2d Camera::screenToWorld(Vector2f position)
{
    //Then apply our inversed projection matrix to go from screen coordinates to world coordinates relative to the camera
    Vector2f world_position = projection_matrix.inverse() * position;
    //Finally apply our own matrix to transform this into global space.
    return getLocalPoint2D(Vector2d(world_position));
}

sp::Ray3d Camera::screenToWorldRay(sp::Vector2f position)
{
    const auto project_inv = projection_matrix.inverse();
    const auto transform = getGlobalTransform();
    return Ray3d(
        Vector3d(transform * (project_inv.multiply(Vector3f(position.x, position.y, 0.0f), 1.0f))),
        Vector3d(transform * (project_inv.multiply(Vector3f(position.x, position.y, 1.0f), 1.0f))));
}

};//namespace sp
