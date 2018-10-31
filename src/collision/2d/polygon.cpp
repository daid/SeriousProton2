#include <sp2/collision/2d/polygon.h>
#include <sp2/logging.h>

#include <private/collision/box2dVector.h>
#include <Box2D/Box2D.h>

namespace sp {
namespace collision {

Polygon2D::Polygon2D()
{
}

void Polygon2D::add(Vector2f point)
{
    points.push_back(point);
}

static bool isRightTurn(Vector2f a, Vector2f b, Vector2f c)
{
	float sum1 = b.x*c.y + a.x*b.y + c.x*a.y;
	float sum2 = b.x*a.y + c.x*b.y + a.x*c.y;

	return (sum1 - sum2) < 0;
}

std::vector<Vector2f> Polygon2D::buildConvex() const
{
    std::vector<Vector2f> result;
    std::vector<Vector2f> input = points;
    
    std::sort(input.begin(), input.end(), [](const Vector2f& a, const Vector2f& b) -> bool
    {
        if (a.x == b.x)
            return a.y < b.y;
        return a.x < b.x;
    });
    
    if (points.size() < 2)
        return result;

    //Build upper half of the hull.
    result.push_back(input[0]);
    result.push_back(input[1]);
    for(unsigned int n=2; n<input.size(); n++)
    {
        result.push_back(input[n]);
        while(result.size() > 2 && !isRightTurn(result[result.size() - 3], result[result.size() - 2], result[result.size() - 1]))
        {
            result.erase(result.end() - 2);
        }
    }

    //Remove the duplicates that the bottom will also add.
    result.erase(result.begin());
    result.erase(result.end() - 1);
    
    //Build the lower half
    unsigned int index0 = result.size();
    result.push_back(input[input.size() - 1]);
    result.push_back(input[input.size() - 2]);
    for(int n=input.size()-1; n>=0; n--)
    {
        result.push_back(input[n]);
        while(result.size() > index0 + 2 && !isRightTurn(result[result.size() - 3], result[result.size() - 2], result[result.size() - 1]))
        {
            result.erase(result.end() - 2);
        }
    }
    return result;
}

void Polygon2D::createFixture(b2Body* body) const
{
    std::vector<Vector2f> points = buildConvex();
    if (points.size() < 3)
    {
        LOG(Warning, "Failed to create convex polygon collision shape, to few points.");
        return;
    }

    while(points.size() > b2_maxPolygonVertices)
    {
        //To many points, delete a point to create a simpler polygon.
        int best = -1;
        float best_length = 0;
        Vector2f p0 = points.back();
        for(unsigned int n=0; n<points.size(); n++)
        {
            Vector2f p1 = points[n];
            float length = (p1 - p0).length();
            if (best == -1 || length < best_length)
            {
                best = n;
                best_length = length;
            }
            p0 = p1;
        }
        points.erase(points.begin() + best);
    }
    
    b2PolygonShape shape;
    b2Vec2 b2points[b2_maxPolygonVertices];
    for(unsigned int n=0; n<points.size(); n++)
        b2points[n] = toVector<float>(points[n]);
    
    shape.Set(b2points, points.size());

    createFixtureOnBody(body, &shape);
}

};//namespace collision
};//namespace sp
