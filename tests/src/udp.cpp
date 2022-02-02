#include "doctest.h"

#include <sp2/io/network/udpSocket.h>

TEST_CASE("UDP_RAW")
{
    sp::io::network::UdpSocket s1;
    sp::io::network::UdpSocket s2;

    CHECK(s1.bind(32032));
    CHECK(s2.send("TEST", 4, sp::io::network::Address("127.0.0.1"), 32032));
    char buffer[10];
    sp::io::network::Address a;
    int p;
    s1.setBlocking(false);
    CHECK(s1.receive(buffer, sizeof(buffer), a, p) == 4);
    CHECK(memcmp(buffer, "TEST", 4) == 0);
}

TEST_CASE("UDP_PACKET")
{
    sp::io::network::UdpSocket s1;
    sp::io::network::UdpSocket s2;
    sp::io::DataBuffer b1(uint32_t(1));
    sp::io::DataBuffer b2;

    CHECK(s1.bind(32032));
    CHECK(s2.send(b1, sp::io::network::Address("127.0.0.1"), 32032));
    sp::io::network::Address a;
    int p;
    s1.setBlocking(false);
    CHECK(s1.receive(b2, a, p));
    CHECK(b1.getDataSize() == b2.getDataSize());
}

TEST_CASE("UDP_MULTICAST")
{
    sp::io::network::UdpSocket s1;
    sp::io::network::UdpSocket s2;

    CHECK(s1.bind(32032));
    CHECK(s1.joinMulticast(10));
    CHECK(s2.sendMulticast("TEST", 4, 10, 32032));
    char buffer[10];
    sp::io::network::Address a;
    int p;
    s1.setBlocking(false);
    CHECK(s1.receive(buffer, sizeof(buffer), a, p) == 4);
    CHECK(memcmp(buffer, "TEST", 4) == 0);
}
