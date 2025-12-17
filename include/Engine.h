#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <stdexcept>
#include <any>

#include "ICommand.h"

class Engine {
public:
    void register_command(const std::string& name, std::shared_ptr<ICommand> cmd) {
        if (!cmd) throw std::invalid_argument("движок не определил команду");
        commands_[name] = std::move(cmd);
    }

    std::any execute(const std::string& name, const ArgMap& args) {
        auto it = commands_.find(name);
        if (it == commands_.end()) {
            throw std::out_of_range("Неизвестная команда: " + name);
        }
        return it->second->invoke(args);
    }

private:
    std::unordered_map<std::string, std::shared_ptr<ICommand>> commands_;
};