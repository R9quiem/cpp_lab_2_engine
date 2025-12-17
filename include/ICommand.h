#pragma once

#include <any>
#include <unordered_map>
#include <string>

using ArgMap = std::unordered_map<std::string, std::any>;

struct ICommand {
    virtual ~ICommand() = default;
    virtual std::any invoke(const ArgMap& args) = 0;
};