#include <sp2/scene/cameraNode.h>

namespace sp {

CameraNode::CameraNode(P<SceneNode> parent)
: SceneNode(parent)
{
    setProjectionMatrix(Matrix4x4d::ortho(-1, 1, -1, 1, -1, 1));
}

};//!namespace sp
