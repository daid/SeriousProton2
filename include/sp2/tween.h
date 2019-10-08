#ifndef SP2_TWEEN_H
#define SP2_TWEEN_H

#include <sp2/graphics/color.h>
#include <sp2/math/vector.h>


namespace sp {

/** Tweening functions.
    Allows for none-linear effects and stuff.
 */
template<typename T> class Tween
{
protected:
    static inline T tweenApply(float f, const T& value0, const T& value1)
    {
        return value0 + (value1 - value0) * f;
    }
public:
    static inline T linear(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        float t = (time_now - time_start) / (time_end - time_start);
        t = std::min(1.0f, std::max(0.0f, t));
        return tweenApply(t, value0, value1);
    }
    
    static inline T easeInQuad(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        float t = (time_now - time_start) / (time_end - time_start);
        t = std::min(1.0f, std::max(0.0f, t));
        return tweenApply(t * t, value0, value1);
    }
    static inline T easeOutQuad(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        float t = (time_now - time_start) / (time_end - time_start);
        t = std::min(1.0f, std::max(0.0f, t));
        return tweenApply(-t * (t - 2.0f), value0, value1);
    }
    static inline T easeInOutQuad(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        float t = (time_now - time_start) / (time_end - time_start);
        t = std::min(1.0f, std::max(0.0f, t));
        return tweenApply(t < 0.5 ? 2*t*t : -1+(4-2*t)*t, value0, value1);
    }
    static inline T easeInCubic(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        float t = (time_now - time_start) / (time_end - time_start);
        t = std::min(1.0f, std::max(0.0f, t));
        return tweenApply(t * t * t, value0, value1);
    }
    static inline T easeOutCubic(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        float t = (time_now - time_start) / (time_end - time_start);
        t = std::min(1.0f, std::max(0.0f, t));
        t -= 1.0;
        return tweenApply((t * t * t + 1), value0, value1);
    }
    static inline T easeInOutCubic(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        float t = (time_now - time_start) / (time_end - time_start);
        t = std::min(1.0f, std::max(0.0f, t));
        return tweenApply(t < 0.5 ? 4*t*t*t : (t-1)*(2*t-2)*(2*t-2)+1, value0, value1);
    }
};

template<> Color Tween<Color>::tweenApply(float f, const Color& value0, const Color& value1);
template<> Vector2d Tween<Vector2d>::tweenApply(float f, const Vector2d& value0, const Vector2d& value1);
template<> Vector3d Tween<Vector3d>::tweenApply(float f, const Vector3d& value0, const Vector3d& value1);

}//namespace sp

#endif//SP2_TWEEN_H
