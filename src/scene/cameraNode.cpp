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

};//!namespace sp
