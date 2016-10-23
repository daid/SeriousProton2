#ifndef SP2_SCENE_CAMERA_NODE_H
#define SP2_SCENE_CAMERA_NODE_H

#include <sp2/scene/node.h>

class b2Body;

namespace sp {

class CameraNode : public SceneNode
{
public:
    CameraNode(P<SceneNode> parent);
    
    void setProjectionMatrix(const sp::Matrix4x4d& matrix) { projection_matrix = matrix; }
    const sp::Matrix4x4d& getProjectionMatrix() { return projection_matrix; }
private:
    sp::Matrix4x4d projection_matrix;
};

};//!namespace sp

#endif//SP2_SCENE_NODE_H
