#define FMT_HEADER_ONLY
#include <ranges>
#include <rich/file.hpp>
#include <rich/regex.hpp>
#include <rich/style.hpp>

void fn() { throw rich::runtime_error("Runtime error occurred!"); }

int main() {
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
