#include <format>
#include <string>
#include <string_view>
#include <slim/common/http/headers.h>
#include <slim/common/http/request.h>
#include <slim/common/http/url.h>

using namespace slim::common;
using namespace slim::common::http;

slim::common::http::Request::Request() {}

slim::common::http::Request::Request(std::string_view _string) {
	slim::SlimValue results = URL::can_parse(_string);
	if(results) {
		__url = URL(_string, results);
		set_headers();
	}
}

slim::common::http::Request::Request(const URL& _url) {
	__url = _url;
	set_headers();
}

const storage_container& slim::common::http::Request::body() const {
	return __body;
}

const slim::ErrorInfo& slim::common::http::Request::get_error() const {
	return __error_info;
}

bool slim::common::http::Request::has_error() const {
	return __error_info.has_error();
}

const Headers& slim::common::http::Request::headers() const {
	return __headers;
}

std::string_view slim::common::http::Request::method() const {
	return __method;
}

void slim::common::http::Request::set_headers() {
	std::string protocol_lower{__url.protocol()};
	std::transform(protocol_lower.begin(), protocol_lower.end(), protocol_lower.begin(), [](unsigned char c){ return std::tolower(c); });
	if(protocol_lower == "http" || protocol_lower == "https") {
		slim::SlimValue results = __headers.set("Host", __url.host());
		if(results) {
			results = __headers.set("Origin", __url.origin());
		}
		if(results.has_error()) {
			__error_info = results.get_error();
		}
	}
}

const URL& slim::common::http::Request::url() const {
	return __url;
}

std::string_view slim::common::http::Request::version() const {
	return __version;
}

const std::string slim::common::http::Request::to_string() const {
	bool add_body = (__body.size() > 0 && __method != "GET" && __method != "DELETE") ? true : false;
	bool has_content_length = false;
	bool has_content_type = false;
	std::string pathname = __url.pathname();
	if(pathname.length() == 0) {
		pathname = "/";
	}
	std::string string_value = std::format("{} {} {}\r\n", __method, pathname, __version);

	for(const auto& [key,value] : __headers.entries() ) {
		string_value += std::format("{}: {}\r\n", key, value);
		std::string key_lower{key};
		std::transform(key_lower.begin(), key_lower.end(), key_lower.begin(), [](unsigned char c){ return std::tolower(c); });

		if(add_body) {
			if(key_lower == "content-length") {
				has_content_length = true;
			}
		}
		else if(!has_content_type) {
			if(key_lower == "content-type") {
				has_content_type = true;
			}
		}
	}

	if(add_body && !has_content_length) {
		string_value += std::format("{}: {}\r\n", "Content-Length", std::to_string(__body.size()));
	}

	if(!has_content_type) {
		string_value += std::format("{}: {}\r\n", "Content-Type", "text/plain; charset=utf-8");
	}

	string_value += "\r\n\r\n";

	if(add_body) {
		string_value += std::format("{}", (char*)__body.data());
	}
	return string_value;
}

