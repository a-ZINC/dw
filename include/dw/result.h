#pragma once

#include <utility>
#include <variant>
#include <string>

namespace dw {

struct Error {
    int code_ = 0;
    std::string message_;

    Error(int code, std::string message) {
        code_ = code;
        message_ = std::move(message);
    }
};

inline std::string to_string(const Error& e) { return e.message_; }
inline const char* to_char(const Error& e) {return e.message_.c_str(); }

template<typename T>
class [[nodiscard]] Result {
public:
    static Result success(T v) { return Result(std::in_place_index<0>, std::move(v)); }
    static Result failure(Error e) { return Result(std::in_place_index<1>, std::move(e)); }
    static Result failure(int code, std::string message) { return failure(Error(code, std::move(message))); }
    
    bool ok() const noexcept { return data_.index() == 0; }
    operator bool() const noexcept { return ok(); }

    T& value() { return std::get<0>(data_); }
    Error& error() { return std::get<1>(data_); }

    T value_or_else(T v) { return ok() ? value() : std::move(v); }

private:
    Result(std::in_place_index_t<0>, T v) : data_(std::in_place_index<0>, std::move(v)){}
    Result(std::in_place_index_t<1>, Error e) : data_(std::in_place_index<1>, std::move(e)){}
    std::variant<T, Error> data_;
};
}