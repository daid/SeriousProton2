#ifndef SP2_SCENE_CAMERA_H
#define SP2_SCENE_CAMERA_H

#include <sp2/scene/node.h>

namespace sp {

class Camera : public Node
{
public:
    enum class Direction
    {
        Horizontal,
        Vertical,
    };

    Camera(P<Node> parent);
    
    void setOrtographic(double view_size = 1.0f, Direction direction=Direction::Vertical, double view_distance=0.0);
    void setPerspective(double field_of_view = 60.0f, Direction direction=Direction::Vertical, double view_distance = 1000.0);
    void setPerspective(double field_of_view = 60.0f, double view_distance = 1000.0);
    
    void setAspectRatio(double ratio);
    const sp::Matrix4x4f& getProjectionMatrix() { return projection_matrix; }

    //Convert a 2d world position to a -1.0 to 1.0 screen view position
    sp::Vector2f worldToScreen(sp::Vector2d position);
    //Convert a 2d screen position (-1.0 to 0.0) to a world position
    sp::Vector2d screenToWorld(sp::Vector2f position);
    //Convert a 2d screen position (-1.0 to 0.0) to a world ray
    sp::Ray3d screenToWorldRay(sp::Vector2f position);
protected:
    void setProjectionMatrix(const sp::Matrix4x4f& matrix) { projection_matrix = matrix; }

    enum class Type
    {
        OrtographicHorizontal,
        OrtographicVertical,
        PerspectiveHorizontal,
        PerspectiveVertical
    };
private:
    Type type;
    double field_of_view;
    double view_distance;
    sp::Matrix4x4f projection_matrix;
};

};//namespace sp

#endif//SP2_SCENE_CAMERA_H
