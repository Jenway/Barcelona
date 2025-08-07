// lib / http / include / http / routing / RouterBuilder.hpp
#pragma once
#include "config/Config.hpp"
#include "http/interfaces/IRequestHandler.hpp"
#include <memory>

namespace http {

class RouterBuilder {
public:
    static auto build(const ServerConfig& config) -> std::unique_ptr<IRequestDispatcher>;
};

} // namespace http