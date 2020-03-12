#include <sp2/io/serialization/serializer.h>
#include "doctest.h"

TEST_CASE("Serializer")
{
    {
        sp::io::serialization::Serializer s("test.data", sp::io::serialization::Serializer::Mode::Write);
        s.set("INT", 1);
        auto d = s.createDataSet("DATA");
        d.set("INT", 2);

        auto l = s.createList("LIST");
        {
            auto a = l.next();
            a.set("INT", 3);
        }
        {
            auto a = l.next();
            a.set("INT", 4);
        }
    }
    {
        sp::io::serialization::Serializer s("test.data", sp::io::serialization::Serializer::Mode::Read);
        CHECK(1 == s.get<int>("INT"));
        const auto& d = s.getDataSet("DATA");
        CHECK(2 == d.get<int>("INT"));
        const auto l = s.getList("LIST");
        CHECK(2 == l.size());
        CHECK(3 == l[0].get<int>("INT"));
        CHECK(4 == l[1].get<int>("INT"));
    }
}
