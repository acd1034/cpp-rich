/// @file cell.hpp
#pragma once
#include <any>
#include <iterator> // std::back_inserter
#include <memory>   // std::shared_ptr

#include <rich/format.hpp>
#include <rich/iterator.hpp> // erased_output
#include <rich/style/line_formatter.hpp>

namespace rich {
  template <typename Char>
  struct cell {
  private:
    std::shared_ptr<void> ptr_ = nullptr;
    std::any lfmtr_{};
    using handler_t = void(std::any*, bool*, std::size_t*, const std::size_t*,
                           erased_output<Char>*, std::size_t*);
    handler_t* handler_ = nullptr;

    void call(bool* b, std::size_t* size = nullptr,
              const std::size_t* in = nullptr,
              erased_output<Char>* str = nullptr,
              std::size_t* out = nullptr) const {
      handler_(&const_cast<std::any&>(lfmtr_), b, size, in, str, out);
    }

  public:
    cell() = default;

    template <line_formattable L, class D = std::remove_cvref_t<L>,
              class LF = line_formatter<D, Char>>
    cell(L&& l)
      : ptr_(std::make_shared<D>(std::forward<L>(l))),
        lfmtr_(std::make_any<LF>(*std::static_pointer_cast<D>(ptr_))),
        handler_([](std::any* lfmtr, bool* b, std::size_t* size,
                    const std::size_t* in, erased_output<Char>* erased,
                    std::size_t* out) {
          if (b != nullptr) {
            *b = bool(*std::any_cast<const LF>(lfmtr));
          } else if (size != nullptr) {
            *size = std::any_cast<const LF>(lfmtr)->formatted_size();
          } else {
            assert(in != nullptr and erased != nullptr and out != nullptr);
            auto result = std::any_cast<LF>(lfmtr)->format_to(*erased, *in);
            *erased = result.out;
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

    template <std::output_iterator<const Char&> Out>
    auto format_to(Out out, const std::size_t n = line_formatter_npos)
      -> fmt::format_to_n_result<Out> {
      erased_output<Char> erased(out);
      std::size_t size;
      call(nullptr, nullptr, &n, &erased, &size);
      out = rich::out<Out>(erased);
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
