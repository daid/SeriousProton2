#include "doctest.h"

#include <sp2/io/network/udpSocket.h>

TEST_CASE("ADDRESS")
{
    sp::io::network::Address a = sp::io::network::Address::getLocalAddress();
    CHECK(a.getHumanReadable().size() > 0);
}
