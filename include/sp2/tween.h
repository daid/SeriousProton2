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
        f = std::min(1.0f, std::max(0.0f, f));
        return value0 + (value1 - value0) * f;
    }
public:
    static inline T linear(float f, const T& value0, const T& value1)
    {
        return tweenApply(f, value0, value1);
    }

    static inline T linear(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        return linear((time_now - time_start) / (time_end - time_start), value0, value1);
    }
    
    static inline T easeInQuad(float f, const T& value0, const T& value1)
    {
        return tweenApply(f * f, value0, value1);
    }

    static inline T easeInQuad(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        return easeInQuad((time_now - time_start) / (time_end - time_start), value0, value1);
    }

    static inline T easeOutQuad(float f, const T& value0, const T& value1)
    {
        return tweenApply(-f * (f - 2.0f), value0, value1);
    }
    
    static inline T easeOutQuad(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        return easeOutQuad((time_now - time_start) / (time_end - time_start), value0, value1);
    }
    
    static inline T easeInOutQuad(float f, const T& value0, const T& value1)
    {
        return tweenApply(f < 0.5f ? 2*f*f : -1+(4-2*f)*f, value0, value1);
    }

    static inline T easeInOutQuad(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        return easeInOutQuad((time_now - time_start) / (time_end - time_start), value0, value1);
    }

    static inline T easeInCubic(float f, const T& value0, const T& value1)
    {
        return tweenApply(f * f * f, value0, value1);
    }

    static inline T easeInCubic(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        return easeInCubic((time_now - time_start) / (time_end - time_start), value0, value1);
    }

    static inline T easeOutCubic(float f, const T& value0, const T& value1)
    {
        f -= 1.0f;
        return tweenApply((f * f * f + 1), value0, value1);
    }

    static inline T easeOutCubic(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        return easeOutCubic((time_now - time_start) / (time_end - time_start), value0, value1);
    }

    static inline T easeInOutCubic(float f, const T& value0, const T& value1)
    {
        return tweenApply(f < 0.5f ? 4*f*f*f : (f-1)*(2*f-2)*(2*f-2)+1, value0, value1);
    }

    static inline T easeInOutCubic(float time_now, float time_start, float time_end, const T& value0, const T& value1)
    {
        return easeInOutCubic((time_now - time_start) / (time_end - time_start), value0, value1);
    }
};

template<> Color Tween<Color>::tweenApply(float f, const Color& value0, const Color& value1);
template<> Vector2d Tween<Vector2d>::tweenApply(float f, const Vector2d& value0, const Vector2d& value1);
template<> Vector3d Tween<Vector3d>::tweenApply(float f, const Vector3d& value0, const Vector3d& value1);

}//namespace sp

#endif//SP2_TWEEN_H
