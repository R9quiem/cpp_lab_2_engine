#pragma once

#include <tuple>
#include <vector>
#include <utility>
#include <type_traits>
#include <stdexcept>

#include "ICommand.h"
#include "ArgSpec.h"


// вспомогательные функции
template <class... Args, std::size_t... I>
std::tuple<Args...> build_tuple_impl(
    const std::vector<ArgSpec>& specs,
    const ArgMap& args,
    std::index_sequence<I...>
) {
    auto get_one = [&](std::size_t idx, auto tag) -> decltype(tag) {
        using T = decltype(tag);
        const auto& spec = specs.at(idx);

        auto it = args.find(spec.name);
        if (it != args.end()) {
            if (it->second.type() != typeid(T)) {
                throw std::invalid_argument("Неправильный тип аргумента '" + spec.name + "'");
            }
            return std::any_cast<T>(it->second);
        }

        if (!spec.has_default()) {
            throw std::invalid_argument("Нету требуемого аргумента '" + spec.name + "'");
        }
        if (spec.default_value.type() != typeid(T)) {
            throw std::invalid_argument("Тип по умолчанию неправильный'" + spec.name + "'");
        }
        return std::any_cast<T>(spec.default_value);
    };

    return std::tuple<Args...>{ get_one(I, Args{})... };
}

template <class... Args>
std::tuple<Args...> build_tuple(const std::vector<ArgSpec>& specs, const ArgMap& args) {
    if (specs.size() != sizeof...(Args)) {
        throw std::logic_error("число ArgSpec != арность функции");
    }
    return build_tuple_impl<Args...>(specs, args, std::index_sequence_for<Args...>{});
}

/*
Пояснение:

Obj - обьект класса, чьи методы мы оборачиваем
R - тип возвращаемого знач. метода
...Args - набор типов аргументов метода

*/
template <class Obj, class R, class... Args>
class MethodWrapper final : public ICommand {
public:
    using Method = R (Obj::*)(Args...);

    MethodWrapper(Obj* obj, Method m, std::vector<ArgSpec> specs)
        : obj_(obj), method_(m), specs_(std::move(specs)) {}

    std::any invoke(const ArgMap& args) override {
        auto tup = build_tuple<Args...>(specs_, args);

        if constexpr (std::is_void_v<R>) {
            std::apply([&](auto&&... xs) { (obj_->*method_)(std::forward<decltype(xs)>(xs)...); }, tup);
            return std::any{};
        } else {
            R res = std::apply([&](auto&&... xs) -> R {
                return (obj_->*method_)(std::forward<decltype(xs)>(xs)...);
            }, tup);
            return std::any{std::move(res)};
        }
    }

private:
    Obj* obj_;
    Method method_;
    std::vector<ArgSpec> specs_;
};
