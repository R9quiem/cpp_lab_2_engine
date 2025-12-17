#pragma once

#include <any>
#include <string>
#include <typeindex>

struct ArgSpec {
    std::string name;
    std::type_index type;
    std::any default_value; 

    bool has_default() const { return default_value.has_value(); }
};