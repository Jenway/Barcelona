// in lib/http/include/HttpStatus.hpp
#pragma once

#include <cstdint>
#include <magic_enum/magic_enum.hpp>
#include <string>

namespace http {

// HTTP Status Codes based on RFC 7231 and other relevant RFCs.
// The type is uint16_t to accommodate all standard codes.
enum class StatusCode : uint16_t {
    // --- 1xx Informational ---
    Continue = 100,
    SwitchingProtocols = 101,

    // --- 2xx Success ---
    Ok = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,

    // --- 3xx Redirection ---
    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304, // 非常重要，用于缓存
    UseProxy = 305,
    TemporaryRedirect = 307,
    PermanentRedirect = 308, // 新增的，推荐用于永久重定向

    // --- 4xx Client Error ---
    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PayloadTooLarge = 413,
    UriTooLong = 414,
    UnsupportedMediaType = 415,
    ExpectationFailed = 417,
    UpgradeRequired = 426,

    // --- 5xx Server Error ---
    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    HTTPVersionNotSupported = 505
};

// 辅助函数，将驼峰式的原因短语转换为带空格的标题
// "NotFound" -> "Not Found"
// "InternalServerError" -> "Internal Server Error"
// "NonAuthoritativeInformation" -> "Non-Authoritative Information"
inline std::string reasonPhraseFromEnum(std::string_view enum_name)
{
    if (enum_name.empty()) {
        return "Unknown Status";
    }

    std::string phrase;
    phrase.reserve(enum_name.length() + 5);
    phrase += enum_name[0]; // 先添加首字母

    for (size_t i = 1; i < enum_name.length(); ++i) {
        if (std::isupper(enum_name[i]) != 0) {
            // 如果前一个字符是小写 (例如 ...otFound)
            // 或者 后一个字符是小写且前一个也是大写 (例如 ...HTTPVersion...)
            if ((std::islower(enum_name[i - 1]) != 0) || (i + 1 < enum_name.length() && (std::islower(enum_name[i + 1]) != 0) && (std::isupper(enum_name[i - 1]) != 0))) {
                phrase += ' ';
            }
        }
        phrase += enum_name[i];
    }
    return phrase;
}

} // namespace http