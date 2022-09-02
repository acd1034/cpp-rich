/// @file cell.hpp
#pragma once
#include <any>
#include <iterator> // std::back_inserter
#include <memory>   // std::shared_ptr

#include <rich/format.hpp>
#include <rich/style/line_formatter.hpp>

namespace rich {
  template <typename Char>
  struct cell {
  private:
    std::shared_ptr<void> ptr_ = nullptr;
    std::any lfmtr_{};
    using handler_t = void(std::any*, bool*, std::size_t*, const std::size_t*,
                           std::basic_string<Char>*, std::size_t*);
    handler_t* handler_ = nullptr;

    void call(bool* b, std::size_t* size = nullptr,
              const std::size_t* in = nullptr,
              std::basic_string<Char>* str = nullptr,
              std::size_t* out = nullptr) const {
      handler_(&const_cast<std::any&>(lfmtr_), b, size, in, str, out);
    }

  public:
    cell() = default;

    template <line_formattable L, class D = std::remove_cvref_t<L>,
              class LF = line_formatter<D, Char>>
    explicit cell(L&& l)
      : ptr_(std::make_shared<D>(std::forward<L>(l))),
        lfmtr_(std::make_any<LF>(*std::static_pointer_cast<D>(ptr_))),
        handler_([](std::any* lfmtr, bool* b, std::size_t* size,
                    const std::size_t* in, std::basic_string<Char>* str,
                    std::size_t* out) {
          using DF = std::remove_cvref_t<LF>;
          if (b != nullptr) {
            *b = bool(*std::any_cast<const DF>(lfmtr));
          } else if (size != nullptr) {
            *size = std::any_cast<const DF>(lfmtr)->formatted_size();
          } else {
            assert(in != nullptr and str != nullptr and out != nullptr);
            auto result = std::any_cast<DF>(lfmtr)->format_to(
              std::back_inserter(*str), *in);
            *out = result.size;
          }
        }) {}

    explicit operator bool() const {
      assert(lfmtr_.has_value());
      bool b;
      call(&b);
      return b;
    }

    std::size_t formatted_size() const {
      assert(lfmtr_.has_value());
      std::size_t size;
      call(nullptr, &size);
      return size;
    }

    auto format(const std::size_t in = line_formatter_npos) {
      assert(lfmtr_.has_value());
      std::basic_string<Char> str{};
      std::size_t out;
      call(nullptr, nullptr, &in, &str, &out);
      return std::make_pair(std::move(str), std::move(out));
    }

    template <std::output_iterator<const Char&> Out>
    auto format_to(Out out, const std::size_t n = line_formatter_npos)
      -> fmt::format_to_n_result<Out> {
      const auto [str, size] = format(n);
      out = rich::copy_to(out, std::basic_string_view<Char>(str));
      return {out, size};
    }
  };
} // namespace rich

template <typename Char>
struct rich::line_formatter<rich::cell<Char>, Char> {
private:
  cell<Char> cell_{};

public:
  line_formatter() = default;

  explicit line_formatter(const cell<Char>& l) : cell_(l) {}

  constexpr explicit operator bool() const { return bool(cell_); }

  constexpr std::size_t formatted_size() const {
    return cell_.formatted_size();
  }

  template <std::output_iterator<const Char&> Out>
  auto format_to(Out out, const std::size_t n = line_formatter_npos)
    -> fmt::format_to_n_result<Out> {
    return cell_.format_to(out, n);
  }
};

template <typename Char>
struct fmt::formatter<rich::cell<Char>, Char>
  : rich::line_formattable_default_formatter<rich::cell<Char>, Char> {};
