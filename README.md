<a href="https://codeberg.org/greergan/SlimTS">
  <img src="https://raw.githubusercontent.com/greergan/SlimTS/master/assets/slimts_logo.png" width="75" alt="SlimTS Logo">
</a>   

# SlimCommonHttpRequest

A lightweight, RFC-oriented HTTP request representation in modern C++.  
Acts as a parsing, building, and serializing backing store for the [SlimTS](https://codeberg.org/greergan/SlimTS) Javascript Request object.  
Part of the [SlimCommon](https://codeberg.org/greergan/SlimCommon) library.  
Built using [SlimLibraryPackager](https://codeberg.org/greergan/SlimLibraryPackager).  
CI/CD supplied by unified workflows provided by [SlimLibraryPackager](https://codeberg.org/greergan/SlimLibraryPackager).

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Default Values](#default-values)
- [Core API](#core-api)
  - [Request class](#request-class)
  - [Constructors and object lifetime](#constructors-and-object-lifetime)
  - [Setters](#setters)
  - [Getters](#getters)
  - [Header population](#header-population)
  - [Serialization](#serialization)
- [Building](#building)
- [Dependencies](#dependencies)
  - [required_packages](#required_packages)
- [Examples](#examples)

## Overview

This library provides a request-line and header builder/parser with:
- Parsing of a raw request string into method, URL, version, and headers
- Construction directly from a [`URL`](https://codeberg.org/greergan/SlimCommonHttpUrl) object
- Automatic `Host` / `Origin` header population for `http`/`https` URLs
- Automatic `Content-Length` and `Content-Type` defaulting on serialization
- Explicit status reporting via [`ErrorStatus`](https://codeberg.org/greergan/SlimCommonHttp) and exception-based failure for constructors
- Minimal runtime overhead via preallocated, single-pass string building on serialize

[↑ Top](#table-of-contents)

## Features

| Feature | Description |
|--------|-------------|
| Parsing | Raw request string parsed via [`URL::can_parse`](https://codeberg.org/greergan/SlimCommonHttpUrl) |
| Construction from URL | Build a `Request` directly from an existing `URL` object |
| Header auto-population | `Host` and `Origin` set automatically for `http`/`https` requests |
| Body | Stored as raw `std::vector<uint8_t>`, no encoding assumed |
| Content-Length | Added automatically on serialize when a body is present and the method allows one |
| Content-Type | Defaults to `text/plain; charset=utf-8` if not explicitly set |
| Pathname fallback | Empty pathname serializes as `/` |
| Error model | `ErrorStatus`-based internally; constructors throw `UrlParseException` / `HttpHeaderException` on failure |

[↑ Top](#table-of-contents)

## Default Values

| Field | Default | Notes |
|-------|---------|-------|
| `method_` | `"GET"` | Used as-is unless explicitly set via `method(std::string_view)` |
| `version_` | `"HTTP/1.1"` | Used as-is unless explicitly set via `version(std::string_view)` |
| `headers_` | empty `Headers` | Populated automatically by `set_headers()` for `http`/`https` URLs (see [Header population](#header-population)) |
| `url_` | default-constructed `URL` | Empty/unset until provided via constructor |
| `body_` | empty `std::vector<uint8_t>` | No body bytes by default |
| `Content-Length` (header, on serialize) | omitted | Only added if `body_` is non-empty, `method_` is not `GET`/`DELETE`, and `Content-Length` is not already set |
| `Content-Type` (header, on serialize) | `text/plain; charset=utf-8` | Only added if `Content-Type` is not already set |
| Pathname (on serialize) | `/` | Substituted when `url_.pathname_` is empty |

Note: `method()` and `version()` setters are unchecked (`void`, `noexcept`) — unlike `Cookie`'s `ErrorStatus`-returning setters, no validation is performed on the values passed in, and defaults are only ever overwritten, not restored.

[↑ Top](#table-of-contents)

## Core API

### Request class

```cpp
slim::common::http::Request r;
```

[↑ Top](#table-of-contents)

### Constructors and object lifetime

| Form | Description |
|------|-------------|
| `Request()` | Default constructor. `method_` = `"GET"`, `version_` = `"HTTP/1.1"`, empty `url_`, `headers_`, and `body_` |
| `Request(std::string_view s)` | Parse a raw request string. Calls `URL::can_parse`, throws `UrlParseException` on bad input; then calls `set_headers()`, throwing `HttpHeaderException` on failure |
| `Request(URL& url)` | Construct from an existing `URL`. Calls `set_headers()` inline; any non-`OK` status is currently not surfaced from this constructor |

[↑ Top](#table-of-contents)

### Setters

| Method | Description |
|--------|-------------|
| `void method(std::string_view) noexcept` | Set the HTTP method. Unvalidated |
| `void version(std::string_view) noexcept` | Set the HTTP version string. Unvalidated |

[↑ Top](#table-of-contents)

### Getters

| Method | Returns |
|--------|---------|
| `const std::vector<uint8_t>& body() const noexcept` | Request body bytes |
| `const Headers& headers() const noexcept` | Header collection |
| `std::string_view method() const noexcept` | HTTP method |
| `const URL& url() const noexcept` | Request URL |
| `std::string_view version() const noexcept` | HTTP version string |

[↑ Top](#table-of-contents)

### Header population

```cpp
ErrorStatus Request::set_headers();
```

Called automatically by both non-default constructors. For `http` and `https` URLs (case-insensitive comparison via `iequals` from [SlimCommonUtilities](https://codeberg.org/greergan/SlimCommonUtilities)):
- Sets `Host` from `url_.host_`
- Sets `Origin` from `url_.origin_`

For any other protocol, `set_headers()` is a no-op and returns `ErrorStatus::OK`.

[↑ Top](#table-of-contents)

### Serialization

```cpp
std::string Request::serialize() const;
// -> "GET /path HTTP/1.1\r\nContent-Type: text/plain; charset=utf-8\r\nHost: example.com\r\n..."
```

Builds the request line (`METHOD pathname VERSION\r\n`), then conditionally adds `Content-Length` and `Content-Type` (see [Default Values](#default-values)), then appends the serialized header block. Buffer is pre-reserved at 16384 bytes to avoid reallocation during the build.

[↑ Top](#table-of-contents)

## Building

This library is built using [SlimLibraryPackager](https://codeberg.org/greergan/SlimLibraryPackager). See that repository for build instructions.

[↑ Top](#table-of-contents)

## Dependencies

### required_packages

External package dependencies for this library are declared in the [`required_packages`](required_packages) file at the repository root. This file is read by [SlimLibraryPackager](https://codeberg.org/greergan/SlimLibraryPackager) during the build process to resolve dependencies and install them if not present.

```
# PackageName [minVersion [maxVersion]]
SlimCommonHttp
SlimCommonUtilities
SlimCommonHttpCookie
SlimCommonHttpCookieStore
SlimCommonHttpHeader
SlimCommonHttpHeaders
SlimCommonHttpUrl 0.9.1
```

- [SlimCommonHttp](https://codeberg.org/greergan/SlimCommonHttp)
- [SlimCommonUtilities](https://codeberg.org/greergan/SlimCommonUtilities)
- [SlimCommonHttpCookie](https://codeberg.org/greergan/SlimCommonHttpCookie)
- [SlimCommonHttpCookieStore](https://codeberg.org/greergan/SlimCommonHttpCookieStore)
- [SlimCommonHttpHeader](https://codeberg.org/greergan/SlimCommonHttpHeader)
- [SlimCommonHttpHeaders](https://codeberg.org/greergan/SlimCommonHttpHeaders)
- [SlimCommonHttpUrl](https://codeberg.org/greergan/SlimCommonHttpUrl) (>= 0.9.1)

[↑ Top](#table-of-contents)

## Examples

```cpp
// Default construction, then explicit setup
slim::common::http::Request r;
r.method("POST");
r.version("HTTP/1.1");
```

```cpp
// Parse a raw request string
try {
    slim::common::http::Request r("https://example.com/api/login");
    auto out = r.serialize();
    // Host and Origin headers are populated automatically
}
catch (const slim::common::http::UrlParseException& e) {
    std::cerr << "URL parse failed: " << e.what() << '\n';
}
catch (const slim::common::http::HttpHeaderException& e) {
    std::cerr << "Header setup failed: " << e.what() << '\n';
}
```

```cpp
// Construct from an existing URL
slim::common::http::URL url("https://example.com/widgets");
slim::common::http::Request r(url);

r.method("GET");
auto header_block = r.serialize();
// -> "GET /widgets HTTP/1.1\r\nContent-Type: text/plain; charset=utf-8\r\nHost: example.com\r\nOrigin: https://example.com\r\n"
```

[↑ Top](#table-of-contents)
