#include <algorithm>
#include <span>
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

Request::Request(const URL& url) : url_(url) {
    auto error = set_headers();
    if (error != ErrorStatus::OK) {
        throw HttpHeaderException(error);
    }
}

std::span<uint8_t> Request::get_body() noexcept {
    if (body_offset_ >= 0) {
        return std::span<uint8_t>(body_).subspan(static_cast<size_t>(body_offset_));
    }
    return {};
}

ErrorStatus Request::parse() noexcept {
    std::string_view s(reinterpret_cast<const char*>(body_.data()), body_.size());
    if (s.empty()) {
        return ErrorStatus::RequestStringEmpty;
    }

    enum class State { Method, Path, Version, VersionCR, HeaderLineStart, BlankLineCR, HeaderName, HeaderValue, HeaderValueCR };

    State            state       = State::Method;
    std::size_t      token_start = 0;
    std::string_view header_name;
    bool             headers_done = false;

    std::size_t pos = 0;
    for (; pos < s.size() && !headers_done; ++pos) {
        const char c = s[pos];

        switch (state) {
        case State::Method:
            if (c == ' ') {
                if (pos == token_start) {
                    return ErrorStatus::RequestStatusLineMalformed;
                }
                method_     = s.substr(token_start, pos - token_start);
                token_start = pos + 1;
                state       = State::Path;
            }
            else if (c == '\r' || c == '\n') {
                return ErrorStatus::RequestStatusLineMalformed;
            }
            break;

        case State::Path:
            if (c == ' ') {
                if (pos == token_start) {
                    return ErrorStatus::RequestStatusLineMalformed;
                }

                try {
                    url_ = URL(s.substr(token_start, pos - token_start), UrlParseMode::PATH);
                }
                catch(const UrlParseException& e) {
                    return e.error();
                }

                token_start = pos + 1;
                state       = State::Version;
            }
            else if (c == '\r' || c == '\n') {
                return ErrorStatus::RequestStatusLineMalformed;
            }
            break;

        case State::Version:
            if (c == '\r') {
                if (pos == token_start) {
                    return ErrorStatus::RequestStatusLineMalformed;
                }
                version_ = s.substr(token_start, pos - token_start);
                state    = State::VersionCR;
            }
            else if (c == ' ') {
                return ErrorStatus::RequestStatusLineMalformed;
            }
            break;

        case State::VersionCR:
            if (c != '\n') {
                return ErrorStatus::RequestStatusLineInvalid;
            }
            state = State::HeaderLineStart;
            break;

        case State::HeaderLineStart:
            if (c == '\r') {
                state = State::BlankLineCR;
            }
            else {
                token_start = pos;
                state       = State::HeaderName;
            }
            break;

        case State::BlankLineCR:
            if (c != '\n') {
                return ErrorStatus::RequestHeadersNotTerminated;
            }
            headers_done = true;
            break;

        case State::HeaderName:
            if (c == ':') {
                if (pos == token_start) {
                    return ErrorStatus::HeaderDelimiterInvalid;
                }
                header_name = s.substr(token_start, pos - token_start);
                token_start = pos + 1;
                state       = State::HeaderValue;
            }
            else if (c == '\r' || c == '\n') {
                return ErrorStatus::HeaderDelimiterInvalid;
            }
            break;

        case State::HeaderValue:
            if (c == '\r') {
                std::string_view value = s.substr(token_start, pos - token_start);
                auto             error = headers_.set(header_name, value);
                if (error != ErrorStatus::OK) {
                    return error;
                }
                state = State::HeaderValueCR;
            }
            break;

        case State::HeaderValueCR:
            if (c != '\n') {
                return ErrorStatus::RequestHeadersNotTerminated;
            }
            state = State::HeaderLineStart;
            break;
        }
    }

    if (!headers_done) {
        // Relies on State enum order: Method, Path, Version, VersionCR all precede HeaderLineStart.
        const bool status_line_incomplete = state < State::HeaderLineStart;
        return status_line_incomplete ? ErrorStatus::RequestStatusLineInvalid : ErrorStatus::RequestHeadersNotTerminated;
    }

    if (pos < s.size()) {
        body_offset_ = static_cast<int>(pos);
    }

    return ErrorStatus::OK;
}

ErrorStatus Request::set_headers() {
    if (iequals(url_.protocol(), "http") || iequals(url_.protocol(), "https")) {
        if (!headers_.has("host")) {
            if (!url_.host().empty()) {
                return headers_.set("Host", url_.host());
            }
            else {
                return ErrorStatus::UrlHostMissing;
            }
        }
    }
    return ErrorStatus::OK;
}

std::string Request::serialize() const {
    constexpr std::size_t initial_buffer_size = 16384;
    const bool has_body = !body_.empty() && method_ != "GET" && method_ != "DELETE";

    std::string result;

    try {
        result.reserve(initial_buffer_size);

        result += method_;
        result += " ";
        result += url_.pathname().empty() ? "/" : url_.pathname();
        result += " ";
        result += version_;
        result += "\r\n";

        if (has_body) {
            if (!headers_.has("Content-Length")) {
                result += "Content-Length: ";
                result += std::to_string(body_.size());
                result += "\r\n";
            }

            if (!headers_.has("Content-Type")) {
                result += "Content-Type: text/plain; charset=utf-8\r\n";
            }
        }

        result += headers_.serialize();
    }
    // probably needs to catch other Slim releated exceptions
    catch (const std::bad_alloc&) {
        throw HttpHeaderException(ErrorStatus::BadAllocation);
    }

    return result;
}

} // namespace slim::common::http
