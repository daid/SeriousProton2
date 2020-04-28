#include <sp2/graphics/color.h>
#include <sp2/stringutil/convert.h>
#include "doctest.h"

TEST_CASE("color")
{
    sp::HsvColor hsv(sp::Color(1, 1, 1));
    CHECK(hsv.saturation == 0.0f);
    CHECK(hsv.value == 100.0f);

    sp::HsvColor red(sp::Color(1, 0, 0));
    CHECK(red.hue == 0.0f);
    CHECK(red.saturation == 100.0f);
    CHECK(red.value == 100.0f);

    CHECK(sp::Color(0xFF0000).r == 0);
    CHECK(sp::Color(0xFF0000).g == 0);
    CHECK(sp::Color(0xFF0000).b == 1);
    CHECK(sp::Color(0xFF0000).a == 0);

    CHECK(sp::stringutil::convert::toColor("#FF0000").r == 1);
    CHECK(sp::stringutil::convert::toColor("#FF0000").g == 0);
    CHECK(sp::stringutil::convert::toColor("#FF0000").b == 0);
    CHECK(sp::stringutil::convert::toColor("#FF0000").a == 1);

    CHECK(sp::stringutil::convert::toColor("#00FF0080").r == 0);
    CHECK(sp::stringutil::convert::toColor("#00FF0080").g == 1);
    CHECK(sp::stringutil::convert::toColor("#00FF0080").b == 0);
    CHECK(sp::stringutil::convert::toColor("#00FF0080").a == doctest::Approx(0.5).epsilon(0.01));
}

TEST_CASE("HueSaturation")
{
    for(float h=0; h<360; h+=1)
    {
        for(float s=10; s<100; s+=1)
        {
            CAPTURE(h);
            CAPTURE(s);
            float v = 100;
            sp::HsvColor hsv(sp::Color(sp::HsvColor(h, s, v)));
            CHECK(hsv.hue == doctest::Approx(h));
            CHECK(hsv.saturation == doctest::Approx(s));
            CHECK(hsv.value == doctest::Approx(v));
        }
    }
}
