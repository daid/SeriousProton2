#include "doctest.h"

#include <sp2/io/http/request.h>

TEST_CASE("HTTP")
{
    sp::io::http::Request request("daid.eu", 80);

    sp::io::http::Request::Response response = request.get("/");
    CHECK(response.success);
    CHECK(response.status == 200);
}

TEST_CASE("HTTPS")
{
    sp::io::http::Request request("daid.eu", 443);

    sp::io::http::Request::Response response = request.get("/");
    CHECK(response.success);
    CHECK(response.status == 200);
}
