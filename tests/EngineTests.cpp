#include <gtest/gtest.h>
#include <any>
#include <memory>
#include <vector>
#include <string>

#include "Engine.h"
#include "MethodWrapper.h"

struct Subject {
    int last = 0;

    int f3(int a, int b) { return a + b; }
    void ping(int x) { last = x; }
    double mix(int a, double b) { return a + b; }
    std::string join(std::string a, std::string b) { return a + b; }
};

// 0) базовый тест на работоспособность
TEST(Engine, SumWorks) {
    Subject subj;

    auto cmd = std::make_shared<MethodWrapper<Subject,int,int,int>>(
        &subj, &Subject::f3,
        std::vector<ArgSpec>{
            {"arg1", typeid(int), 0},
            {"arg2", typeid(int), 0},
        }
    );

    Engine e;
    e.register_command("sum", cmd);

    auto res = e.execute("sum", {{"arg1", 4}, {"arg2", 5}});
    EXPECT_EQ(std::any_cast<int>(res), 9);
}

// 1) Использование значений по умолчанию (передали только один аргумент)
TEST(Engine, UsesDefaultWhenMissing) {
    Subject subj;

    auto cmd = std::make_shared<MethodWrapper<Subject,int,int,int>>(
        &subj, &Subject::f3,
        std::vector<ArgSpec>{
            {"arg1", typeid(int), 10}, // дефолт 10
            {"arg2", typeid(int), 0},
        }
    );

    Engine e;
    e.register_command("sum", cmd);

    auto res = e.execute("sum", {{"arg2", 7}});
    EXPECT_EQ(std::any_cast<int>(res), 17); // 10 + 7
}

// 2) Ошибка: нет обязательного аргумента (нет дефолта)
TEST(Engine, ThrowsOnMissingRequiredArg) {
    Subject subj;

    auto cmd = std::make_shared<MethodWrapper<Subject,int,int,int>>(
        &subj, &Subject::f3,
        std::vector<ArgSpec>{
            {"arg1", typeid(int), std::any{}},
            {"arg2", typeid(int), 0},
        }
    );

    Engine e;
    e.register_command("sum", cmd);

    EXPECT_THROW(
        e.execute("sum", {{"arg2", 5}}),
        std::invalid_argument
    );
}

// 3) Ошибка: неправильный тип переданного аргумента 
TEST(Engine, ThrowsOnTypeMismatch) {
    Subject subj;

    auto cmd = std::make_shared<MethodWrapper<Subject,double,int,double>>(
        &subj, &Subject::mix,
        std::vector<ArgSpec>{
            {"a", typeid(int), 0},
            {"b", typeid(double), 0.0},
        }
    );

    Engine e;
    e.register_command("mix", cmd);

    EXPECT_THROW(
        e.execute("mix", {{"a", 1}, {"b", 2}}),
        std::invalid_argument
    );
}

// 4) Возврат другого типа (double)
TEST(Engine, ReturnsDouble) {
    Subject subj;

    auto cmd = std::make_shared<MethodWrapper<Subject,double,int,double>>(
        &subj, &Subject::mix,
        std::vector<ArgSpec>{
            {"a", typeid(int), 0},
            {"b", typeid(double), 0.0},
        }
    );

    Engine e;
    e.register_command("mix", cmd);

    auto res = e.execute("mix", {{"a", 2}, {"b", 3.5}});
    EXPECT_DOUBLE_EQ(std::any_cast<double>(res), 5.5);
}

// 5) void-метод: результат пустой, но состояние объекта меняется
TEST(Engine, VoidReturnAndStateChange) {
    Subject subj;

    auto cmd = std::make_shared<MethodWrapper<Subject,void,int>>(
        &subj, &Subject::ping,
        std::vector<ArgSpec>{
            {"x", typeid(int), 0},
        }
    );

    Engine e;
    e.register_command("ping", cmd);

    auto res = e.execute("ping", {{"x", 42}});

    EXPECT_FALSE(res.has_value()); 
    EXPECT_EQ(subj.last, 42);
}

// 6) Неизвестная команда
TEST(Engine, ThrowsOnUnknownCommand) {
    Engine e;
    EXPECT_THROW(
        e.execute("no_such_command", {{"x", 1}}),
        std::out_of_range
    );
}

// 7) Лишние аргументы игнорируются
TEST(Engine, ExtraArgsAreIgnored) {
    Subject subj;

    auto cmd = std::make_shared<MethodWrapper<Subject,int,int,int>>(
        &subj, &Subject::f3,
        std::vector<ArgSpec>{
            {"arg1", typeid(int), 0},
            {"arg2", typeid(int), 0},
        }
    );

    Engine e;
    e.register_command("sum", cmd);

   
    auto res = e.execute("sum", {{"arg1", 1}, {"arg2", 2}, {"extra", 999}});
    EXPECT_EQ(std::any_cast<int>(res), 3);
}

// 8) Строки 
TEST(Engine, StringArgsWork) {
    Subject subj;

    auto cmd = std::make_shared<MethodWrapper<Subject,std::string,std::string,std::string>>(
        &subj, &Subject::join,
        std::vector<ArgSpec>{
            {"a", typeid(std::string), std::string("")},
            {"b", typeid(std::string), std::string("")},
        }
    );

    Engine e;
    e.register_command("join", cmd);

    auto res = e.execute("join", {{"a", std::string("ab")}, {"b", std::string("cd")}});
    EXPECT_EQ(std::any_cast<std::string>(res), "abcd");
}

// 9) Ошибка: дефолт задан не тем типом 
TEST(Engine, ThrowsOnBadDefaultType) {
    Subject subj;

    auto cmd = std::make_shared<MethodWrapper<Subject,int,int,int>>(
        &subj, &Subject::f3,
        std::vector<ArgSpec>{
            {"arg1", typeid(int), 0},
            {"arg2", typeid(int), std::string("oops")},
        }
    );

    Engine e;
    e.register_command("sum", cmd);

    EXPECT_THROW(
        e.execute("sum", {{"arg1", 1}}),
        std::invalid_argument
    );
}
