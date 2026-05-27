#include <catch2/catch_test_macros.hpp>
#include <slim/common/http/headers.h>
#include <slim/common/http/request.h>
#include <slim/common/http/url.h>

using namespace slim::common::http;

TEST_CASE("string to Request", "[url to request]") {
	Request request{"http://example.com"};
	auto& headers = request.headers();
	REQUIRE(headers.entries().size() == 2);
	REQUIRE(request.method() == "GET");
	REQUIRE(request.version() == "HTTP/1.1");
	REQUIRE(headers.get("Host") == "example.com");
	REQUIRE(headers.get("Origin") == "http://example.com");
	auto request_string = request.to_string();
	REQUIRE(request_string.starts_with("GET / HTTP/1.1"));
	REQUIRE(request_string.contains("Content-Type: text/plain; charset=utf-8"));
	REQUIRE(request_string.ends_with("\r\n\r\n\r\n"));
}
TEST_CASE("string to Request with / as path", "[url to request]") {
	Request request{"http://example.com/"};
	auto& headers = request.headers();
	REQUIRE(headers.entries().size() == 2);
	REQUIRE(request.method() == "GET");
	REQUIRE(request.version() == "HTTP/1.1");
	REQUIRE(headers.get("Host") == "example.com");
	REQUIRE(headers.get("Origin") == "http://example.com");
	auto request_string = request.to_string();
	REQUIRE(request_string.starts_with("GET / HTTP/1.1"));
	REQUIRE(request_string.contains("Content-Type: text/plain; charset=utf-8"));
	REQUIRE(request_string.ends_with("\r\n\r\n\r\n"));
}
TEST_CASE("string to Request with path", "[url to request]") {
	Request request{"http://example.com/path"};
	auto& headers = request.headers();
	REQUIRE(headers.entries().size() == 2);
	REQUIRE(request.method() == "GET");
	REQUIRE(request.version() == "HTTP/1.1");
	REQUIRE(headers.get("Host") == "example.com");
	REQUIRE(headers.get("Origin") == "http://example.com");
	auto request_string = request.to_string();
	REQUIRE(request_string.starts_with("GET /path HTTP/1.1"));
	REQUIRE(request_string.contains("Content-Type: text/plain; charset=utf-8"));
	REQUIRE(request_string.ends_with("\r\n\r\n\r\n"));
}