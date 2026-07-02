#include <catch2/catch_test_macros.hpp>
#include <slim/common/http/error_codes.h>
#include <slim/common/http/headers.h>
#include <slim/common/http/request.h>
#include <slim/common/http/url.h>

using namespace slim::common::http;

TEST_CASE("Request constructed from a string_view", "[request]") {
    constexpr std::string_view raw =
        "GET / HTTP/1.1\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Host: example.com\r\n"
        "\r\n";

    Request request{};
    std::vector<uint8_t> body(raw.begin(), raw.end());
    request.set_body(std::move(body));
    REQUIRE(request.parse() == ErrorStatus::OK);

    SECTION("parses method") {
        REQUIRE(request.method() == "GET");
    }
    SECTION("parses version") {
        REQUIRE(request.version() == "HTTP/1.1");
    }
    SECTION("parses Host header") {
        REQUIRE(request.headers().get("Host")->get_value()[0] == "example.com");
    }
    SECTION("parses Content-Type header") {
        REQUIRE(request.headers().get("Content-Type")->get_value()[0] == "text/plain");
        REQUIRE(request.headers().get("Content-Type")->get_value()[1] == "charset=utf-8");
    }
    SECTION("parses header count") {
        REQUIRE(request.headers().entries().size() == 2);
    }
}

TEST_CASE("Request constructed from a complex string_view", "[request]") {
    constexpr std::string_view raw =
        "POST /api/v1/users?id=42&active=true HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Content-Length: 27\r\n"
        "Accept: text/html, application/xhtml+xml, application/xml;q=0.9\r\n"
        "Authorization: Bearer abc123.def456.ghi789\r\n"
        "X-Forwarded-For: 203.0.113.5, 70.41.3.18\r\n"
        "Cookie: session_id=xyz; theme=dark\r\n"
        "\r\n"
        "{\"name\":\"Alice\",\"age\":30}";

    Request request{};
    std::vector<uint8_t> body(raw.begin(), raw.end());
    request.set_body(std::move(body));
    REQUIRE(request.parse() == ErrorStatus::OK);

    SECTION("parses method") {
        REQUIRE(request.method() == "POST");
    }
    SECTION("parses version") {
        REQUIRE(request.version() == "HTTP/1.1");
    }
    SECTION("parses path") {
        REQUIRE(request.url().pathname() == "/api/v1/users");
    }
    SECTION("parses query string parameters") {
        REQUIRE(request.url().searchParams()->get("id")->get_value() == "42");
        REQUIRE(request.url().searchParams()->get("active")->get_value() == "true");
    }
    SECTION("parses Host header") {
        REQUIRE(request.headers().get("Host")->get_value()[0] == "example.com");
    }
    SECTION("parses Content-Type header with multiple values") {
        REQUIRE(request.headers().get("Content-Type")->get_value()[0] == "application/json");
        REQUIRE(request.headers().get("Content-Type")->get_value()[1] == "charset=utf-8");
    }
    SECTION("parses Content-Length header") {
        REQUIRE(request.headers().get("Content-Length")->get_value()[0] == "27");
    }
    SECTION("parses Accept header with multiple weighted values") {
        auto accept_values = request.headers().get("Accept")->get_value();
        REQUIRE(accept_values[0] == "text/html");
        REQUIRE(accept_values[1] == "application/xhtml+xml");
        REQUIRE(accept_values[2] == "application/xml;q=0.9");
    }
    SECTION("parses Authorization header") {
        REQUIRE(request.headers().get("Authorization")->get_value()[0] == "Bearer abc123.def456.ghi789");
    }
    SECTION("parses X-Forwarded-For header with multiple IPs") {
        auto forwarded = request.headers().get("X-Forwarded-For")->get_value();
        REQUIRE(forwarded[0] == "203.0.113.5");
        REQUIRE(forwarded[1] == "70.41.3.18");
    }
    SECTION("parses Cookie header") {
        auto cookies = request.headers().get_cookies();
        REQUIRE(cookies->get("session_id")->get_value() == "xyz");
        REQUIRE(cookies->get("theme")->get_value() == "dark");
    }
    SECTION("parses header count") {
        REQUIRE(request.headers().entries().size() == 6);
    }
    SECTION("parses cookie count") {
        REQUIRE(request.headers().get_cookies()->entries().size() == 2);
    }
    SECTION("parses request body") {
        auto b = request.get_body();
        REQUIRE(std::string(b.begin(), b.end()) == "{\"name\":\"Alice\",\"age\":30}");
    }
}

TEST_CASE("Request constructed with edge-case headers and empty body", "[request]") {
    constexpr std::string_view raw =
        "DELETE /resources/9001 HTTP/1.1\r\n"
        "Host: api.example.org\r\n"
        "Content-Type:    text/plain   ;   charset=utf-8   \r\n"
        "X-Empty-Header: \r\n"
        "X-Custom-Flag: true; verbose; debug=1\r\n"
        "\r\n";

    Request request{};
    std::vector<uint8_t> body(raw.begin(), raw.end());
    request.set_body(std::move(body));
    REQUIRE(request.parse() == ErrorStatus::OK);

    SECTION("parses method") {
        REQUIRE(request.method() == "DELETE");
    }
    SECTION("parses path with numeric segment") {
        REQUIRE(request.url().pathname() == "/resources/9001");
    }
    SECTION("trims whitespace around Content-Type values") {
        REQUIRE(request.headers().get("Content-Type")->get_value()[0] == "text/plain");
        REQUIRE(request.headers().get("Content-Type")->get_value()[1] == "charset=utf-8");
    }
    SECTION("parses header with empty value") {
        REQUIRE(request.headers().get("X-Empty-Header")->get_value().empty());
    }
    SECTION("parses header with multiple semicolon-delimited flags") {
        auto flags = request.headers().get("X-Custom-Flag")->get_value();
        REQUIRE(flags[0] == "true; verbose; debug=1");
    }
    SECTION("body is empty after parse when none provided") {
        REQUIRE(request.get_body().empty());
    }
}

TEST_CASE("Request constructed from a URL", "[request]") {
    SECTION("from URL&, no path") {
        auto url = URL("http://example.com");
        Request request{url};
        request.method("GET");
        request.version("HTTP/1.1");
        auto& headers = request.headers();
        REQUIRE(headers.entries().size() == 1);
        REQUIRE(headers.get("Host")->get_value()[0] == "example.com");
        auto request_string = request.serialize();
        std::string expected =
            "GET / HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "\r\n";
        REQUIRE(request_string == expected);
   }

    SECTION("from URL&, root path") {
        auto url = URL("http://example.com/");
        Request request{url};
        request.method("GET");
        request.version("HTTP/1.1");
        auto& headers = request.headers();

        REQUIRE(headers.entries().size() == 1);
        REQUIRE(headers.get("Host")->get_value()[0] == "example.com");

        auto request_string = request.serialize();
        REQUIRE(request_string.starts_with("GET / HTTP/1.1"));
        REQUIRE(request_string.ends_with("\r\n\r\n"));
    }

    SECTION("from URL&, with path") {
        auto url = URL("http://example.com/path");
        Request request{url};
        request.method("GET");
        request.version("HTTP/1.1");
        auto& headers = request.headers();

        REQUIRE(headers.entries().size() == 1);
        REQUIRE(headers.get("Host")->get_value()[0] == "example.com");

        auto request_string = request.serialize();
        REQUIRE(request_string.starts_with("GET /path HTTP/1.1"));
        REQUIRE(request_string.ends_with("\r\n\r\n"));
    }
}
