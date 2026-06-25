#include <catch2/catch_test_macros.hpp>
#include <slim/common/http/headers.h>
#include <slim/common/http/request.h>
#include <slim/common/http/url.h>

using namespace slim::common::http;

TEST_CASE("Request constructed from a URL", "[request]") {
    SECTION("from std::string_view, no path") {
        Request request{"http://example.com"};
        request.method("GET");
        request.version("HTTP/1.1");
        auto& headers = request.headers();

        REQUIRE(headers.entries().size() == 2);
        REQUIRE(headers.has("Host"));
        REQUIRE(headers.has("Origin"));
        REQUIRE(headers.get("Host")->get_value()[0] == "example.com");
        REQUIRE(headers.get("Origin")->get_value()[0] == "http://example.com");

        auto request_string = request.serialize();
        REQUIRE(request_string.starts_with("GET / HTTP/1.1"));
        REQUIRE(request_string.contains("Content-Type: text/plain; charset=utf-8"));
        REQUIRE(request_string.ends_with("\r\n\r\n"));
    }

    SECTION("from std::string_view, root path") {
        Request request{"http://example.com/"};
        request.method("GET");
        request.version("HTTP/1.1");
        auto& headers = request.headers();

        REQUIRE(headers.entries().size() == 2);
        REQUIRE(headers.get("Host")->get_value()[0] == "example.com");
        REQUIRE(headers.get("Origin")->get_value()[0] == "http://example.com");

        auto request_string = request.serialize();
        REQUIRE(request_string.starts_with("GET / HTTP/1.1"));
        REQUIRE(request_string.contains("Content-Type: text/plain; charset=utf-8"));
        REQUIRE(request_string.ends_with("\r\n\r\n"));
    }

    SECTION("from std::string_view, with path") {
        Request request{"http://example.com/path"};
        request.method("GET");
        request.version("HTTP/1.1");
        auto& headers = request.headers();

        REQUIRE(headers.entries().size() == 2);
        REQUIRE(headers.get("Host")->get_value()[0] == "example.com");
        REQUIRE(headers.get("Origin")->get_value()[0] == "http://example.com");

        auto request_string = request.serialize();
        REQUIRE(request_string.starts_with("GET /path HTTP/1.1"));
        REQUIRE(request_string.contains("Content-Type: text/plain; charset=utf-8"));
        REQUIRE(request_string.ends_with("\r\n\r\n"));
    }

    SECTION("from URL&, no path") {
        auto url = URL("http://example.com");
        Request request{url};
        request.method("GET");
        request.version("HTTP/1.1");
        auto& headers = request.headers();

        REQUIRE(headers.entries().size() == 2);
        REQUIRE(headers.get("Host")->get_value()[0] == "example.com");
        REQUIRE(headers.get("Origin")->get_value()[0] == "http://example.com");

        auto request_string = request.serialize();
        REQUIRE(request_string.starts_with("GET / HTTP/1.1"));
        REQUIRE(request_string.contains("Content-Type: text/plain; charset=utf-8"));
        REQUIRE(request_string.ends_with("\r\n\r\n"));
    }

    SECTION("from URL&, root path") {
        auto url = URL("http://example.com/");
        Request request{url};
        request.method("GET");
        request.version("HTTP/1.1");
        auto& headers = request.headers();

        REQUIRE(headers.entries().size() == 2);
        REQUIRE(headers.get("Host")->get_value()[0] == "example.com");
        REQUIRE(headers.get("Origin")->get_value()[0] == "http://example.com");

        auto request_string = request.serialize();
        REQUIRE(request_string.starts_with("GET / HTTP/1.1"));
        REQUIRE(request_string.contains("Content-Type: text/plain; charset=utf-8"));
        REQUIRE(request_string.ends_with("\r\n\r\n"));
    }

    SECTION("from URL&, with path") {
        auto url = URL("http://example.com/path");
        Request request{url};
        request.method("GET");
        request.version("HTTP/1.1");
        auto& headers = request.headers();

        REQUIRE(headers.entries().size() == 2);
        REQUIRE(headers.get("Host")->get_value()[0] == "example.com");
        REQUIRE(headers.get("Origin")->get_value()[0] == "http://example.com");

        auto request_string = request.serialize();
        REQUIRE(request_string.starts_with("GET /path HTTP/1.1"));
        REQUIRE(request_string.contains("Content-Type: text/plain; charset=utf-8"));
        REQUIRE(request_string.ends_with("\r\n\r\n"));
    }
}
