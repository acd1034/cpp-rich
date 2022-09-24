#define FMT_HEADER_ONLY
#include <ranges>
#include <rich/file.hpp>
#include <rich/regex.hpp>
#include <rich/style.hpp>

// This is a comment. Some keywords such as `auto` are contained.
void fn() { throw rich::runtime_error("Rich exception thrown!"); }

int main() {
  try {
    fn();
  } catch (rich::exception& e) {
    auto contents = rich::get_file_contents(e.where().file_name());
    const std::size_t extra = 3;
    auto partial = rich::extract_partial_contents(std::string_view(contents),
                                                  e.where().line(), extra);
    auto lns = rich::lines(rich::syntax_highlight(partial));
    // clang-format off
    auto str = fmt::format("{}:{}:{} in {}",
                           e.where().file_name(), e.where().line(),
                           e.where().column(), e.where().function_name());
    rich::lines<char> lns2{{std::string_view(str), {}}};
    // clang-format on

    { // Editor
      auto pnl = rich::panel(lns);
      pnl.box = rich::box::Editor<char>;
      pnl.contents_spec.width = std::size_t(-1);
      pnl.border_spec.width = 2;
      pnl.title = std::string_view("Traceback (most recent call)");
      fmt::print("{:=^80}\n{}\n", "Editor", pnl);
    }
    { // panel in panel
      rich::panel pnl(lns);
      rich::panel pnl2(pnl, {});
      pnl2.border_spec.style = fg(fmt::terminal_color::magenta);
      pnl2.title = std::string_view("Traceback (most recent call)");
      fmt::print("{:=^80}\n{}\n", "panel in panel", pnl2);
    }
    { // panel with filled border
      rich::panel pnl(lns);
      pnl.border_spec.fill = "$";
      pnl.border_spec.align = rich::align_t::right;
      pnl.border_spec.width = 3;
      rich::panel pnl2(pnl, {});
      pnl2.border_spec.style = fg(fmt::terminal_color::magenta);
      pnl2.border_spec.fill = "$";
      pnl2.border_spec.align = rich::align_t::left;
      pnl2.border_spec.width = 3;
      pnl2.title = std::string_view("Traceback (most recent call)");
      fmt::print("{:=^80}\n{}\n", "panel with filled border", pnl2);
    }
    { // enum without highlight
      rich::enumerate enm(lns);
      enm.start_line = e.where().line() - extra;
      enm.end_line = e.where().line() + extra;
      fmt::print("{:=^80}\n{}\n", "enum without highlight", enm);
    }

    // more practical examples
    rich::enumerate numbered_code(lns);
    numbered_code.start_line = e.where().line() - extra;
    numbered_code.end_line = e.where().line() + extra;
    numbered_code.highlight_line = e.where().line();
    numbered_code.highlight_spec.width = 2;

    { // enum in panel
      rich::panel pnl(numbered_code, {});
      pnl.title = std::string_view("Traceback (most recent call)");
      fmt::print("{:=^80}\n{}\n", "enum in panel", pnl);
    }
    { // table of lines and enum
      rich::table tbl(lns2, numbered_code);
      tbl.title = std::string_view("Traceback (most recent call)");
      fmt::print("{:=^80}\n{}\n", "table of lines and enum", tbl);
    }
    { // RoundedNoSep table
      rich::table tbl(lns2, numbered_code);
      tbl.box = rich::box::RoundedNoSep<char>;
      tbl.title = std::string_view("Traceback (most recent call)");
      fmt::print("{:=^80}\n{}\n", "RoundedNoSep table", tbl);
    }
    { // NoBorder table
      rich::table tbl(lns2, numbered_code);
      tbl.box = rich::box::NoBorder<char>;
      tbl.border_spec.width = 0;
      tbl.title = std::string_view("Traceback (most recent call)");
      tbl.nomatter = true;
      fmt::print("{:=^80}\n{}\n", "NoBorder table", tbl);
    }
  }
}
