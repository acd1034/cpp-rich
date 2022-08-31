/// @file exception.hpp
#pragma once
#include <exception>
#include <source_location>
#include <string>

#include <rich/fundamental.hpp>

namespace rich {
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/exception
  // https://github.com/llvm/llvm-project/blob/main/libcxx/include/stdexcept
  // https://github.com/llvm/llvm-project/tree/main/libcxx/src/support/runtime
  // https://stackoverflow.com/questions/28640553/exception-class-with-a-char-constructor
  struct exception : std::exception {
  private:
    std::string msg_{};
    std::source_location loc_{};

  public:
    explicit exception(const std::string& msg,
                       std::source_location loc = std::source_location::current())
      : msg_(msg), loc_(loc) {}
    explicit exception(const char* msg,
                       std::source_location loc = std::source_location::current())
      : msg_(msg), loc_(loc) {}

    exception(const exception&) noexcept = default;
    exception& operator=(const exception&) noexcept = default;

    ~exception() noexcept override = default;
    const char* what() const noexcept override {
      return msg_.c_str();
    }
    std::source_location where() const noexcept {
      return loc_;
    }
  };

  // 雑に使える例外
  struct runtime_error : exception {
    using exception::exception;
  };
} // namespace rich
