#include <algorithm>
#include <string>
#include <string_view>

#include <slim/common/http/error_codes.h>
#include <slim/common/http/headers.h>
#include <slim/common/http/request.h>
#include <slim/common/http/url.h>
#include <slim/common/utilities.h>

namespace slim::common::http {

namespace {
    using slim::common::utilities::iequals;
} // namespace

Request::Request(std::string_view s) {
    hints_map hints;

    auto error = URL::can_parse(s, hints);
    if (error != ErrorStatus::OK) throw UrlParseException(error);

    url_ = URL(s, hints);

    error = set_headers();
    if (error != ErrorStatus::OK) throw HttpHeaderException(error);
}

ErrorStatus Request::set_headers() {
    if (iequals(url_.protocol_, "http") || iequals(url_.protocol_, "https")) {
        auto error = headers_.set("Host", url_.host_);
        if (error != ErrorStatus::OK) return error;
        return headers_.set("Origin", url_.origin_);
    }
    return ErrorStatus::OK;
}

std::string Request::serialize() const {
    constexpr std::size_t initial_buffer_size = 16384;

    const bool add_body = !body_.empty() && method_ != "GET" && method_ != "DELETE";

    std::string pathname{url_.pathname_};
    if (pathname.empty()) pathname = "/";

    std::string result;
    result.reserve(initial_buffer_size);

    result += method_;
    result += " ";
    result += pathname;
    result += " ";
    result += version_;
    result += "\r\n";

    if (add_body && !headers_.has("Content-Length")) {
        result += "Content-Length: ";
        result += std::to_string(body_.size());
        result += "\r\n";
    }
    if (!headers_.has("Content-Type")) result += "Content-Type: text/plain; charset=utf-8\r\n";
    result += headers_.serialize();

    return result;
}

} // namespace slim::common::http
