#define FMT_HEADER_ONLY
#include <ranges>
#include <rich/file.hpp>
#include <rich/regex.hpp>
#include <rich/style.hpp>

void fn() { throw rich::runtime_error("Runtime error occurred!"); }

int main() {
  const std::regex re(R"((//.*?\n)|\b(auto|const|int|void|throw)\b|(".*?"))");
  const std::vector<fmt::text_style> styles{
    fmt::emphasis::faint,          // comment
    fg(fmt::terminal_color::red),  // keyword
    fg(fmt::terminal_color::blue), // literal
    fg(fmt::color::red),
  };
  const auto highlight = std::views::transform([&styles](const auto& x) {
    const auto& [pre, mo] = x;
    if (!mo)
      return rich::segment(pre);
    const auto n = rich::match_find(*mo);
    if (!n or *n >= std::ranges::size(styles))
      return rich::segment(pre, rich::ranges::back(styles));
    return rich::segment(pre, rich::ranges::index(styles, *n));
  });

  try {
    fn();
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
    rich::enumerate numbered_code(rich::regex_range(partial, re) | highlight);
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
