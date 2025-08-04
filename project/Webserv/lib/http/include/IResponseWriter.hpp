#pragma once
#include "ISinker.hpp"
#include "Message.hpp"

namespace http {
class IResponseWriter {
public:
    virtual ~IResponseWriter() = default;
    virtual auto writeTo(ISinker& sinker) -> std::expected<core::WriteResult, std::error_code> = 0;
    virtual void bind_to(http::Response) = 0;
    [[nodiscard]] virtual bool isKeepAlive() const = 0;
    virtual void reset() = 0;
};
}
