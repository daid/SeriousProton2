#include <sp2/stringutil/i18n.h>
#include <sp2/attributes.h>
#include <sp2/io/directoryResourceProvider.h>
#include "doctest.h"

TEST_CASE("i18n")
{
    sp::io::DirectoryResourceProvider resource_provider("resources");
    sp::i18n::reset();
    CHECK(sp::tr("X") == "X");
    CHECK(sp::i18n::load("non-existsing.po") == false);
    CHECK(sp::i18n::load("test.po") == true);
    CHECK(sp::tr("X") == "Y");
    CHECK(sp::tr("Z") == "Z");
    CHECK(sp::tr("context", "X") == "YY");
    sp::i18n::reset();
    CHECK(sp::tr("X") == "X");
    CHECK(sp::i18n::load("test.mo") == true);
    CHECK(sp::tr("X") == "Y");
    CHECK(sp::tr("context", "X") == "YY");
}
