#ifndef SP2_SCENE_CAMERA_NODE_H
#define SP2_SCENE_CAMERA_NODE_H

#include <sp2/scene/node.h>

class b2Body;

namespace sp {

class CameraNode : public SceneNode
{
public:
    CameraNode(P<SceneNode> parent);
    
    void setOrtographic(double view_distance = 1.0f);
    void setPerspective(double field_of_view = 60.0f, double view_distance = 1000.0);
    
    void setAspectRatio(double ratio);
    const sp::Matrix4x4d& getProjectionMatrix() { return projection_matrix; }

protected:
    void setProjectionMatrix(const sp::Matrix4x4d& matrix) { projection_matrix = matrix; }

    enum class Type
    {
        Ortographic,
        Perspective
    };
private:
    Type type;
    double field_of_view;
    double view_distance;
    sp::Matrix4x4d projection_matrix;
};

};//!namespace sp

#endif//SP2_SCENE_NODE_H
