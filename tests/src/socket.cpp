#include <sp2/io/network/tcpSocket.h>
#include <sp2/io/http/websocket.h>
#include "doctest.h"

#include <chrono>
#include <thread>

TEST_CASE("TcpSocket")
{
    sp::io::network::TcpSocket socket;

#ifdef EMSCRIPTEN
    CHECK(socket.connect(sp::io::network::Address("daid.eu"), 80) == false);
    CHECK(socket.connect(sp::io::network::Address("echo.websocket.org"), 80) == false);
#else
    CHECK(socket.connect(sp::io::network::Address("daid.eu"), 80));
    char buffer[128];
    sprintf(buffer, "GET / HTTP/1.1\r\nHost: daid.eu\r\n\r\n");
    socket.send(buffer, strlen(buffer));
    CHECK(socket.receive(buffer, sizeof(buffer)) == sizeof(buffer));
#endif
}


TEST_CASE("WebSocket")
{
#ifdef EMSCRIPTEN
    //TODO: Emscripten requires main loop updates before websockets work.
#else
    sp::io::http::Websocket socket;
    CHECK(socket.connect("ws://echo.websocket.org/"));
    CHECK(socket.isConnecting() == true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    CHECK(socket.isConnected() == true);
#endif
}
