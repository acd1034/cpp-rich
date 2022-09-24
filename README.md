# `cpp-rich`: Text formatting with decoration

[![Linux build status](https://github.com/acd1034/cpp-rich/actions/workflows/linux-build.yml/badge.svg)](https://github.com/acd1034/cpp-rich/actions/workflows/linux-build.yml)
[![Github issues](https://img.shields.io/github/issues/acd1034/cpp-rich)](https://github.com/acd1034/cpp-rich/issues)
[![Github forks](https://img.shields.io/github/forks/acd1034/cpp-rich)](https://github.com/acd1034/cpp-rich/network/members)
[![Github stars](https://img.shields.io/github/stars/acd1034/cpp-rich)](https://github.com/acd1034/cpp-rich/stargazers)
[![Github top language](https://img.shields.io/github/languages/top/acd1034/cpp-rich)](https://github.com/acd1034/cpp-rich/)
[![Github license](https://img.shields.io/github/license/acd1034/cpp-rich)](https://github.com/acd1034/cpp-rich/)

<!-- [![macOS build status](https://github.com/acd1034/cpp-rich/actions/workflows/macos-build.yml/badge.svg)](https://github.com/acd1034/cpp-rich/actions/workflows/macos-build.yml) -->

<!-- Click [here](https://acd1034.github.io/cpp-rich/index.html) to see the HTML documentation generated by Doxygen. -->

> `cpp-rich` は C++でリッチテキストを出力するためのライブラリです。色付き出力、行数付き出力、枠線、表 (現時点では一列のみ) に対応しています。
> - 📄 [Link to single header file](https://raw.githubusercontent.com/acd1034/cpp-rich/single-header/rich.hpp)
> - 💪 [Try it out on Compiler Explorer!](https://godbolt.org/z/Phb7WbvEx)

## Quick example

```cpp
#define FMT_HEADER_ONLY
#include <rich/rich.hpp>

int divide(int num, int div) {
  if (div == 0)
    throw rich::runtime_error("Division by zero");
  return num / div;
}

int main() {
  try {
    divide(1, 0);
  } catch (rich::exception& e) {
    // location
    auto str = fmt::format("{}:{}:{} in {}",
                           e.where().file_name(), e.where().line(),
                           e.where().column(), e.where().function_name());
    rich::lines<char> location{{std::string_view(str), {}}};

    // numbered_code
    auto contents = rich::get_file_contents(e.where().file_name());
    const std::size_t extra = 3;
    auto partial = rich::extract_partial_contents(std::string_view(contents),
                                                  e.where().line(), extra);
    rich::enumerate numbered_code(rich::syntax_highlight(partial));
    numbered_code.start_line = e.where().line() - extra;
    numbered_code.end_line = e.where().line() + extra;
    numbered_code.highlight_line = e.where().line();
    numbered_code.highlight_spec.width = 2;

    // message
    rich::lines<char> message{{e.what(), {}}};

    // table
    rich::table table(location, numbered_code, message);
    table.title = std::string_view("Traceback (most recent call)");

    fmt::print("{}\n", table);
  }
}
```

<img alt="cpp-rich-light" src="https://user-images.githubusercontent.com/48613285/192085521-87090ce8-d7fc-4085-bf11-d350d4c99f4d.png">

## Library Dependencies

This library uses [fmt](https://github.com/fmtlib/fmt) for formatting texts.
