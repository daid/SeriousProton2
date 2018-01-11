#ifndef SP2_SCENE_CAMERA_H
#define SP2_SCENE_CAMERA_H

#include <sp2/scene/node.h>

namespace sp {

class Camera : public Node
{
public:
    Camera(P<Node> parent);
    
    void setOrtographic(double view_distance = 1.0f, bool fixed_width=false);
    void setPerspective(double field_of_view = 60.0f, double view_distance = 1000.0);
    
    void setAspectRatio(double ratio);
    const sp::Matrix4x4d& getProjectionMatrix() { return projection_matrix; }

    sp::Vector2f worldToScreen(sp::Vector2d position);
    sp::Vector2d screenToWorld(sp::Vector2f position);
protected:
    void setProjectionMatrix(const sp::Matrix4x4d& matrix) { projection_matrix = matrix; }

    enum class Type
    {
        OrtographicFixedHeight,
        OrtographicFixedWidth,
        Perspective
    };
private:
    Type type;
    double field_of_view;
    double view_distance;
    sp::Matrix4x4d projection_matrix;
};

};//!namespace sp

#endif//SP2_SCENE_CAMERA_H
