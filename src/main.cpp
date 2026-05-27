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
}

slim::common::http::Request::Request(const URL& _url) {
	__url = _url;
	__headers.set("Host", _url.host());
}

const storage_container& slim::common::http::Request::body() const {
	return __body;
}

const Headers& slim::common::http::Request::headers() const {
	return __headers;
}

std::string_view slim::common::http::Request::method() const {
	return __method;
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

	std::string string_value = std::format("{} {} {}\r\n", __method, __url.pathname(), __version);

	for(const auto& [key,value] : __headers.entries() ) {
		string_value += std::format("{}:{}\r\n", key, value);
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
		string_value += std::format("{}:{}\r\n", "Content-Length", std::to_string(__body.size()));
	}

	if(!has_content_type) {
		string_value += std::format("{}:{}\r\n", "Content-Type", "text/plain; charset=utf-8");
	}

	string_value += "\r\n\r\n";

	if(add_body) {
		string_value += std::format("{}", (char*)__body.data());
	}
	return string_value;
}

